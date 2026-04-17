#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <fitsio.h>

int main() {
    fitsfile *infptr, *outfptr;
    int status = 0;
    int hdutype;
    long nrows = 0;
    int ncols = 0;
    int class_col;
    char class_val[10];
    char *val_ptr = class_val;

    const char *infile = "Data/SDSS/spAll-v6_1_3.fits.gz";
    const char *outfile = "!Data/SDSS/galaxies.fits.gz"; // '!' overwrites if exists

    // Open source FITS file
    if (fits_open_file(&infptr, infile, READONLY, &status)) {
        fits_report_error(stderr, status);
        return 1;
    }

    // Create the output FITS file
    if (fits_create_file(&outfptr, outfile, &status)) {
        fits_report_error(stderr, status);
        fits_close_file(infptr, &status);
        return 1;
    }

    // 1. Copy primary HDU (HDU 1)
    if (fits_copy_hdu(infptr, outfptr, 0, &status)) {
        fits_report_error(stderr, status);
    }

    // 2. Move to HDU 2 in source (the table data)
    if (fits_movabs_hdu(infptr, 2, &hdutype, &status)) {
        std::cerr << "Error: Could not move to HDU 2." << std::endl;
        fits_report_error(stderr, status);
        fits_close_file(infptr, &status);
        fits_close_file(outfptr, &status);
        return 1;
    }

    fits_get_num_rows(infptr, &nrows, &status);
    fits_get_num_cols(infptr, &ncols, &status);

    // Find the CLASS column index
    if (fits_get_colnum(infptr, CASEINSEN, (char*)"CLASS", &class_col, &status)) {
        std::cerr << "Error: Could not find 'CLASS' column." << std::endl;
        fits_report_error(stderr, status);
        fits_close_file(infptr, &status);
        fits_close_file(outfptr, &status);
        return 1;
    }

    // 3. Create a new table HDU in destination with the same structure
    char **ttype = new char*[ncols];
    char **tform = new char*[ncols];
    char **tunit = new char*[ncols];
    for (int i = 1; i <= ncols; i++) {
        ttype[i-1] = new char[FLEN_VALUE];
        tform[i-1] = new char[FLEN_VALUE];
        tunit[i-1] = new char[FLEN_VALUE];
        fits_get_bcolparms(infptr, i, ttype[i-1], tunit[i-1], tform[i-1], NULL, NULL, NULL, NULL, NULL, &status);
    }

    fits_create_tbl(outfptr, BINARY_TBL, 0, ncols, ttype, tform, tunit, (char*)"GALAXY_DATA", &status);

    // Clean up temporary column arrays
    for (int i = 0; i < ncols; i++) {
        delete[] ttype[i];
        delete[] tform[i];
        delete[] tunit[i];
    }
    delete[] ttype;
    delete[] tform;
    delete[] tunit;

    if (status) {
        fits_report_error(stderr, status);
        fits_close_file(infptr, &status);
        fits_close_file(outfptr, &status);
        return 1;
    }

    // 4. Filtering process
    std::cout << "Starting filtering process..." << std::endl;
    std::cout << "Total rows to process: " << nrows << std::endl;

    long galaxy_count = 0;
    long row_len;
    fits_get_rowsize(infptr, &row_len, &status); // width of a row in bytes
    std::vector<unsigned char> row_buffer(row_len);

    for (long i = 1; i <= nrows; i++) {
        // Read CLASS column for this row
        fits_read_col(infptr, TSTRING, class_col, i, 1, 1, NULL, &val_ptr, NULL, &status);
        
        if (strcmp(class_val, "GALAXY") == 0) {
            // Efficiently copy the whole row binary data
            fits_read_tblbytes(infptr, i, 1, row_len, row_buffer.data(), &status);
            galaxy_count++;
            fits_write_tblbytes(outfptr, galaxy_count, 1, row_len, row_buffer.data(), &status);
        }

        if (i % 100000 == 0) {
            std::cout << "Processed " << i << " rows... Found " << galaxy_count << " galaxies." << std::endl;
        }

        if (status) {
            fits_report_error(stderr, status);
            break;
        }
    }

    // Update the number of rows in the output header
    fits_update_key(outfptr, TLONG, (char*)"NAXIS2", &galaxy_count, NULL, &status);

    std::cout << "Done! Found " << galaxy_count << " galaxies." << std::endl;
    std::cout << "Output saved to: Data/SDSS/galaxies.fits.gz" << std::endl;

    fits_close_file(infptr, &status);
    fits_close_file(outfptr, &status);

    if (status) fits_report_error(stderr, status);

    return 0;
}
