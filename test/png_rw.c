#include <stdio.h>
#include <stdlib.h>
#include "img/png.h"

int png_read_write(const char *in, const char *out, unsigned int alignRow) {
    unsigned int width, height;
    int alpha;
    void *buffer;
    int retval;

    retval = png_read(in, alignRow, &width, &height, &alpha, 0, &buffer);
    if (!retval) {
        return EXIT_FAILURE;
    }

    retval = png_write(out, alignRow, width, height, alpha, 0, buffer);
    if (!retval) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        puts("Usage: png_rw <infile> <outfile>");
        return EXIT_FAILURE;
    }

    return png_read_write(argv[1], argv[2], 0);
}
