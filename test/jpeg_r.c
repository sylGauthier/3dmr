#include <stdio.h>
#include <stdlib.h>

#include <3dmr/img/jpeg.h>

int main(int argc, char** argv) {
    unsigned char* buffer = NULL;
    unsigned int w, h, channels;
    unsigned int reqc = 0, align = 0, rev = 0;

    if (argc < 2) {
        printf("Usage: %s <file>\n", argv[0]);
        return 1;
    }

    for (rev = 0; rev <= 1; rev++) {
        for (reqc = 0; reqc < 6; reqc++) {
            for (align = 0; align <= 8; align += 4) {
                printf("%s, rev = %d, reqc = %d, align = %d: ", argv[1], rev, reqc, align);
                if (!jpeg_read_file(argv[1], align, &w, &h, &channels, reqc, rev, &buffer)) {
                    printf("fail\n");
                } else {
                    printf("pass: w = %d, h = %d, c = %d\n", w, h, channels);
                }
                free(buffer);
                buffer = NULL;
            }
        }
    }
    return 0;
}
