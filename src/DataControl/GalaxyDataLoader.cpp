#include <iostream>
#include <algorithm>
#include <cstring>
#include <iomanip>
#include <fitsio.h>

int main() {
    freopen("results/outputs/Galaxy_output.txt", "w", stdout);
    freopen("results/outputs/Galaxy_errors.txt", "w", stderr);

    fitsfile *fptr;
    int status = 0;
    long nrows = 0;
    int ncols = 0;
    int hdutype;

    // Open FITS file (cfitsio automatically handles .gz)
    fits_open_file(&fptr, "Data/SDSS/galaxies.fits.gz", READONLY, &status);

    if (status) {
        std::cerr << "Error opening FITS file:" << std::endl;
        fits_report_error(stderr, status);
        return 1;
    }

    std::cout << "=== Galaxies FITS File Info ===" << std::endl;

    // Move to HDU 2 (Table Data)
    fits_movabs_hdu(fptr, 2, &hdutype, &status);

    if (status) {
        fits_report_error(stderr, status);
        fits_close_file(fptr, &status);
        return 1;
    }

    // Get table dimensions
    fits_get_num_rows(fptr, &nrows, &status);
    fits_get_num_cols(fptr, &ncols, &status);

    if (status) {
        std::cerr << "Error getting table dimensions:" << std::endl;
        fits_report_error(stderr, status);
        fits_close_file(fptr, &status);
        return 1;
    }

    std::cout << "Total Rows: " << nrows << " | Total Columns: " << ncols << std::endl;

    // Print all column names and types
    std::cout << "\n=== Column Names and Types ===" << std::endl;

    int col_type;
    long repeat, width;

    for (int i = 1; i <= ncols; i++) {
        status = 0;

        char colname[FLEN_VALUE];
        fits_get_bcolparms(fptr, i, colname, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &status);
        fits_get_coltype(fptr, i, &col_type, &repeat, &width, &status);

        std::string type_str;
        switch(col_type) {
            case TBIT:      type_str = "BIT";      break;
            case TBYTE:     type_str = "BYTE";     break;
            case TSHORT:    type_str = "SHORT";    break;
            case TLONG:     type_str = "LONG";     break;
            case TLONGLONG: type_str = "LONGLONG"; break;
            case TFLOAT:    type_str = "FLOAT";    break;
            case TDOUBLE:   type_str = "DOUBLE";   break;
            case TLOGICAL:  type_str = "LOGICAL";  break;
            case TSTRING:   type_str = "STRING";   break;
            default:        type_str = "UNKNOWN";  break;
        }

        std::cout << "Col " << std::setw(3) << i << ": "
                  << std::setw(30) << std::left << colname
                  << " (" << type_str << ")"
                  << " repeat=" << repeat << " width=" << width
                  << std::endl;
    }

    // Read and display data from first 10 rows
    std::cout << "\n=== Data Sample (first 50 rows, all columns) ===" << std::endl;
    long num_display = std::min(50L, nrows);

    for (int col = 1; col <= ncols; col++) {
        status = 0;
        char colname[FLEN_VALUE];
        fits_get_bcolparms(fptr, col, colname, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &status);
        fits_get_coltype(fptr, col, &col_type, &repeat, &width, &status);
        if (status) continue;

        std::cout << "\n--- Col " << col << ": " << colname << " ---" << std::endl;

        if (col_type == TDOUBLE) {
            double *data = new double[num_display];
            status = 0;
            fits_read_col(fptr, TDOUBLE, col, 1, 1, num_display, NULL, data, NULL, &status);
            if (!status)
                for (long i = 0; i < num_display; i++)
                    std::cout << std::scientific << std::setprecision(6) << data[i] << std::endl;
            else
                std::cout << "Error reading column" << std::endl;
            delete[] data;
        }
        else if (col_type == TFLOAT) {
            float *data = new float[num_display];
            status = 0;
            fits_read_col(fptr, TFLOAT, col, 1, 1, num_display, NULL, data, NULL, &status);
            if (!status)
                for (long i = 0; i < num_display; i++)
                    std::cout << std::scientific << std::setprecision(6) << data[i] << std::endl;
            else
                std::cout << "Error reading column" << std::endl;
            delete[] data;
        }
        else if (col_type == TLONG || col_type == TLONGLONG) {
            long *data = new long[num_display];
            status = 0;
            fits_read_col(fptr, TLONG, col, 1, 1, num_display, NULL, data, NULL, &status);
            if (!status)
                for (long i = 0; i < num_display; i++)
                    std::cout << data[i] << std::endl;
            else
                std::cout << "Error reading column" << std::endl;
            delete[] data;
        }
        else if (col_type == TSHORT) {
            short *data = new short[num_display];
            status = 0;
            fits_read_col(fptr, TSHORT, col, 1, 1, num_display, NULL, data, NULL, &status);
            if (!status)
                for (long i = 0; i < num_display; i++)
                    std::cout << data[i] << std::endl;
            else
                std::cout << "Error reading column" << std::endl;
            delete[] data;
        }
        else if (col_type == TBYTE) {
            unsigned char *data = new unsigned char[num_display];
            status = 0;
            fits_read_col(fptr, TBYTE, col, 1, 1, num_display, NULL, data, NULL, &status);
            if (!status)
                for (long i = 0; i < num_display; i++)
                    std::cout << (int)data[i] << std::endl;
            else
                std::cout << "Error reading column" << std::endl;
            delete[] data;
        }
        else if (col_type == TSTRING) {
            char **data = new char*[num_display];
            for (long i = 0; i < num_display; i++) {
                data[i] = new char[width + 1];
                memset(data[i], 0, width + 1);
            }
            status = 0;
            fits_read_col(fptr, TSTRING, col, 1, 1, num_display, NULL, data, NULL, &status);
            if (!status)
                for (long i = 0; i < num_display; i++)
                    std::cout << data[i] << std::endl;
            else
                std::cout << "Error reading column" << std::endl;
            for (long i = 0; i < num_display; i++)
                delete[] data[i];
            delete[] data;
        }
        else {
            std::cout << "(unsupported type)" << std::endl;
        }
    }

    // Close FITS file
    fits_close_file(fptr, &status);

    if (status) {
        std::cerr << "Error closing file:" << std::endl;
        fits_report_error(stderr, status);
    }

    return 0;
}
