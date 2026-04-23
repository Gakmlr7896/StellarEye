#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <cmath>
#include <fitsio.h>

int main() {
    fitsfile *infptr, *outfptr;
    int status = 0;
    int hdutype;
    long nrows = 0;
    int ncols = 0;
    int class_col, ra_col, dec_col;
    
    const char *infile = "Data/SDSS/spAll-v6_1_3.fits.gz";
    const char *outfile = "!Data/SDSS/galaxies.fits"; 

    std::cout << "Opening " << infile << "..." << std::endl;
    if (fits_open_file(&infptr, infile, READONLY, &status)) {
        fits_report_error(stderr, status);
        return 1;
    }

    if (fits_create_file(&outfptr, outfile, &status)) {
        fits_report_error(stderr, status);
        fits_close_file(infptr, &status);
        return 1;
    }

    fits_copy_hdu(infptr, outfptr, 0, &status);
    fits_movabs_hdu(infptr, 2, &hdutype, &status);
    fits_get_num_rows(infptr, &nrows, &status);
    fits_get_num_cols(infptr, &ncols, &status);

    fits_get_colnum(infptr, CASEINSEN, (char*)"CLASS", &class_col, &status);
    fits_get_colnum(infptr, CASEINSEN, (char*)"PLUG_RA", &ra_col, &status);
    fits_get_colnum(infptr, CASEINSEN, (char*)"PLUG_DEC", &dec_col, &status);

    fits_copy_hdu(infptr, outfptr, 0, &status);
    fits_delete_rows(outfptr, 1, nrows, &status);

    std::cout << "Filtering " << nrows << " rows. Validating coordinates..." << std::endl;

    long out_row = 0;
    char class_val[20];
    char *val_ptr = class_val;
    double ra_val, dec_val;

    for (long i = 1; i <= nrows; i++) {
        fits_read_col(infptr, TSTRING, class_col, i, 1, 1, NULL, &val_ptr, NULL, &status);
        fits_read_col(infptr, TDOUBLE, ra_col, i, 1, 1, NULL, &ra_val, NULL, &status);
        fits_read_col(infptr, TDOUBLE, dec_col, i, 1, 1, NULL, &dec_val, NULL, &status);

        std::string c(class_val);
        size_t last = c.find_last_not_of(' ');
        if (std::string::npos != last) c = c.substr(0, last + 1);

        // More robust check: isfinite (excludes NaN and Inf) and non-zero
        if (c == "GALAXY" && 
            std::isfinite(ra_val) && ra_val != 0.0 &&
            std::isfinite(dec_val) && dec_val != 0.0) {
            
            out_row++;
            fits_copy_rows(infptr, outfptr, i, 1, &status);
        }

        if (i % 200000 == 0) {
            std::cout << "Processed " << i << " rows. Found " << out_row << " clean galaxies." << std::endl;
        }
        
        if (status) {
            fits_report_error(stderr, status);
            break;
        }
    }

    fits_close_file(infptr, &status);
    fits_close_file(outfptr, &status);

    std::system("gzip -f Data/SDSS/galaxies.fits");
    std::cout << "Done! Total Clean Galaxies: " << out_row << std::endl;
    return 0;
}
