#include <stdio.h>
#include <stdlib.h>
#include <game/img/png.h>

int png_read_write(const char* in, const char* out, unsigned int alignRow) {
    unsigned int width, height, channels;
    unsigned char* buffer;
    int retval;

    if (!png_read(in, alignRow, &width, &height, &channels, 0, 0, &buffer)) {
        return 0;
    }

    retval = png_write(out, alignRow, width, height, channels, 0, buffer);
    free(buffer);
    return retval;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        puts("Usage: png_rw <infile> <outfile>");
        return EXIT_FAILURE;
    }

    return !png_read_write(argv[1], argv[2], 0);
}
