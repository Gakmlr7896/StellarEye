import os 

url = "https://dr19.sdss.org/sas/dr19/spectro/boss/redux/v6_1_3/spAll-v6_1_3.fits.gz"
filename = os.path.basename(url)
save_path = os.path.join("Data/SDSS", filename)

os.makedirs("Data/SDSS", exist_ok=True)
os.system(f"wget {url} -O {save_path}")

# Don't forget to add -c to the wget command if you want to resume a download that was interrupted.
# os.system(f"wget {url} -c -O {save_path}")
