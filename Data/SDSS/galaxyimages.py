import os
import requests
from astropy.io import fits
import time
from concurrent.futures import ThreadPoolExecutor
import shutil

# Configuration
FITS_PATH = "Data/SDSS/galaxies.fits.gz"
BASE_OUTPUT_DIR = "Data/SDSS/GalaxyImages"

# OPTIMIZED FOR CNN TRAINING:
# Scale 0.1 provides a high-detail zoom on the target galaxy.
# 256x256 is the standard input size for many CNN architectures (like ResNet or EfficientNet).
SCALE = 0.1 
WIDTH = 256
HEIGHT = 256
MAX_WORKERS = 20 

def get_dir_and_filename(index, ra, dec):
    """Organizes files into subfolders and uses a descriptive filename."""
    subfolder = str(index // 1000).zfill(3)
    directory = os.path.join(BASE_OUTPUT_DIR, subfolder)
    # Filename includes RA/DEC to ensure coordinate mapping is transparent
    filename = os.path.join(directory, f"gal_{index}_ra{ra:.3f}_dec{dec:.3f}.jpg")
    return directory, filename

def download_image(ra, dec, index):
    directory, filename = get_dir_and_filename(index, ra, dec)
    
    if os.path.exists(filename):
        return 0 

    os.makedirs(directory, exist_ok=True)
    
    # SDSS Cutout API centers the image on ra/dec automatically
    url = f"https://skyserver.sdss.org/dr16/SkyServerWS/ImgCutout/getjpeg?ra={ra}&dec={dec}&scale={SCALE}&width={WIDTH}&height={HEIGHT}"
    
    try:
        response = requests.get(url, timeout=10)
        if response.status_code == 200:
            # Check if we got an actual image and not a 'no data' placeholder (small file size)
            if len(response.content) < 2000:
                return -3
            with open(filename, "wb") as f:
                f.write(response.content)
            return 1 
        return -1 
    except:
        return -2 

def main():
    total, used, free = shutil.disk_usage(".")
    if free < 15 * 1024**3:
        print(f"Warning: Low disk space ({free // 1024**3}GB free).")

    print(f"Opening clean catalog: {FITS_PATH}...")
    try:
        with fits.open(FITS_PATH) as hdul:
            data = hdul[1].data
            total_rows = len(data)
            print(f"Starting optimized downloads for {total_rows} galaxies...")

            start_time = time.time()
            success_count = 0
            
            with ThreadPoolExecutor(max_workers=MAX_WORKERS) as executor:
                futures = [executor.submit(download_image, data['PLUG_RA'][i], data['PLUG_DEC'][i], i) 
                           for i in range(total_rows)]
                
                for i, future in enumerate(futures):
                    res = future.result()
                    if res == 1: success_count += 1
                    
                    if i % 100 == 0 and i > 0:
                        elapsed = time.time() - start_time
                        speed = i / elapsed
                        eta = (total_rows - i) / speed if speed > 0 else 0
                        print(f"Progress: {i}/{total_rows} | New: {success_count} | Speed: {speed:.1f} img/s | ETA: {eta/3600:.1f}h")

        print(f"Complete! All images saved to {BASE_OUTPUT_DIR}")
    except Exception as e:
        print(f"An error occurred: {str(e)}")

if __name__ == "__main__":
    main()
