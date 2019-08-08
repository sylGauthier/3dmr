#include <stdio.h>
#include <stdlib.h>
#include <game/img/hdr.h>
#include <game/img/png.h>

int main(int argc, char** argv) {
    float* hdr;
    unsigned char* ldr;
    unsigned int width, height;
    int ret = 1;

    if (argc != 3) {
        printf("Usage: %s input.hdr output.png\n", argv[0]);
        return 1;
    }

    if (hdr_read(argv[1], 4, &width, &height, &hdr)) {
        if ((ldr = hdr_to_ldr(hdr, width, height, 4, 1, 2.2f, 1.0f))) {
            ret = !png_write(argv[2], 4, width, height, 4, 0, ldr);
            free(ldr);
        }
        free(hdr);
    }

    return ret;
}
