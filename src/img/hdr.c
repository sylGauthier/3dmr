#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <math.h>

struct HdrHeader {
    unsigned int width, height;
    enum HdrFlags {
        X_MAJOR = 1,
        X_FLIP = 2,
        Y_FLIP = 4,
        FMT_XYZ = 8
    } flags;
};

#define IS_EOL(ptr) ((ptr)[*(ptr) == '\r'] == '\n')

static int hdr_parse_header(FILE* f, struct HdrHeader* header) {
    char buffer[2048], *ptr, *end;
    int hasWidth = 0, hasHeight = 0, done = 0;
    int isX;

    if (!fgets(buffer, sizeof(buffer), f)
     || ((strncmp(buffer, "#?RADIANCE", 10) || !IS_EOL(buffer + 10))
      && (strncmp(buffer, "#?RGBE", 6) || !IS_EOL(buffer + 6)))) {
        return 0;
    }

    header->flags = 0;
    do {
        if (!fgets(buffer, sizeof(buffer), f)) {
            return 0;
        }
        if (IS_EOL(buffer)) {
            done = 1;
        } else if (buffer[0] == '#') {
            continue;
        } else if (!strncmp(buffer, "FORMAT=", 7)) {
            if (!strncmp(buffer + 7, "32-bit_rle_rgbe", 15) && IS_EOL(buffer + 7 + 15)) {
                header->flags &= ~FMT_XYZ;
            } else if (!strncmp(buffer + 7, "32-bit_rle_xyze", 15) && IS_EOL(buffer + 7 + 15)) {
                header->flags |= FMT_XYZ;
            } else {
                return 0;
            }
        }
    } while (!done);

    if (!fgets(buffer, sizeof(buffer), f)) {
        return 0;
    }
    ptr = buffer;
    do {
        if (*ptr == '-' || *ptr == '+') {
            if ((isX = (ptr[1] == 'X')) || ptr[1] == 'Y') {
                long val = strtol(ptr + 2, &end, 10);
                if (end <= ptr + 2 || val <= 0 || val > ((long)INT_MAX) || !isspace(*end)) {
                    return 0;
                }
                if (isX) {
                    if (hasWidth) {
                        return 0;
                    } else if (!hasHeight) {
                        header->flags |= X_MAJOR;
                    }
                    hasWidth = 1;
                    header->width = val;
                    header->flags |= ((*ptr == '-') * X_FLIP);
                } else {
                    if (hasHeight) {
                        return 0;
                    }
                    hasHeight = 1;
                    header->height = val;
                    header->flags |= ((*ptr == '+') * Y_FLIP);
                }
                for (ptr = end + 1; *ptr && isspace(*ptr); ptr++);
            } else {
                return 0;
            }
        } else {
            return 0;
        }
    } while (!(hasWidth && hasHeight));

    return !*ptr;
}

static int hdr_read_scanline_newstyle(FILE* f, unsigned char* scanline, unsigned int lenScanline) {
    unsigned int i, c;
    unsigned char code, val, *ptr;

    for (c = 0; c < 4; c++) {
        ptr = scanline + c;
        for (i = 0; i < lenScanline; ) {
            if (fread(&code, 1, 1, f) != 1) {
                return 0;
            }
            if (code > 0x80) { /* according to the spec ("high-order bit set") it should be (code & 0x80), but the reference implementation (freadcolrs()) does this */
                if (fread(&val, 1, 1, f) != 1) {
                    return 0;
                }
                for (code &= 0x7F; code && i < lenScanline; code--) {
                    *ptr = val;
                    ptr += 4;
                    i++;
                }
            } else {
                while (code && i < lenScanline) {
                    if (fread(ptr, 1, 1, f) != 1) {
                        return 0;
                    }
                    ptr += 4;
                    i++;
                    code--;
                }
            }
        }
    }

    return 1;
}

static int hdr_read_scanline(FILE* f, unsigned char* scanline, unsigned int lenScanline) {
    unsigned char* ptr = scanline;
    unsigned long repeat;
    unsigned int shift = 0;

    if (fread(ptr, 1, 4, f) != 4) {
        return 0;
    }
    if (ptr[0] == 2 && ptr[1] == 2 && !(ptr[2] & 0x80) && (((unsigned int)(ptr[2]) << 8) | (unsigned int)(ptr[3])) == lenScanline) {
        return hdr_read_scanline_newstyle(f, scanline, lenScanline);
    }
    do {
        if (ptr[0] == 1 && ptr[1] == 1 && ptr[2] == 1) {
            if (ptr == scanline || shift > 24) {
                return 0;
            }
            for (repeat = (ptr[3] << shift); repeat && lenScanline; repeat--) {
                memcpy(ptr, ptr - 4, 4);
                ptr += 4;
                lenScanline--;
            }
            shift += 8;
        } else {
            ptr += 4;
            lenScanline--;
            shift = 0;
        }
    } while (lenScanline && fread(ptr, 1, 4, f) == 4);

    return !lenScanline;
}

static int hdr_read_content(FILE* f, const struct HdrHeader* header, unsigned int alignRows, float** buffer) {
    unsigned char *scanline = NULL, *src;
    float *dest, factor;
    unsigned int i, j, x, y, *major, *minor, majorFlip, minorFlip, numScanlines, lenScanline, destRowSize, padding;

    *buffer = NULL;
    if (header->flags & X_MAJOR) {
        numScanlines = header->width;
        lenScanline = header->height;
        major = &x;
        minor = &y;
        majorFlip = X_FLIP;
        minorFlip = Y_FLIP;
    } else  {
        numScanlines = header->height;
        lenScanline = header->width;
        major = &y;
        minor = &x;
        majorFlip = Y_FLIP;
        minorFlip = X_FLIP;
    }
    if (header->width > UINT_MAX / (3 * sizeof(float))) {
        return 0;
    }
    destRowSize = 3 * sizeof(float) * header->width;
    padding = (alignRows - (destRowSize % alignRows)) % alignRows;
    if (destRowSize > UINT_MAX - padding || (destRowSize += padding) > UINT_MAX / header->height
     || !(*buffer = malloc(destRowSize * header->height))
     || lenScanline > UINT_MAX / 4
     || !(scanline = malloc(lenScanline * 4))) {
        free(*buffer);
        free(scanline);
        return 0;
    }
    destRowSize /= sizeof(float);

    for (i = 0; i < numScanlines; i++) {
        if (!hdr_read_scanline(f, scanline, lenScanline)) {
            break;
        }
        *major = (header->flags & majorFlip) ? (numScanlines - 1 - i) : i;
        src = scanline;
        for (j = 0; j < lenScanline; j++) {
            *minor = (header->flags & minorFlip) ? (lenScanline - 1 - j) : j;
            dest = *buffer + destRowSize * y + 3 * x;
            if (src[3]) {
                factor = ldexp(1.0, (int)(src[3]) - (int)(128 + 8));
                dest[0] = (float)(src[0]) * factor;
                dest[1] = (float)(src[1]) * factor;
                dest[2] = (float)(src[2]) * factor;
            } else {
                dest[0] = dest[1] = dest[2] = 0.0f;
            }
            src += 4;
        }
    }

    free(scanline);
    if (i < numScanlines) {
        free(*buffer);
        return 0;
    }
    return 1;
}

int hdr_read(const char* filename, unsigned int alignRows, unsigned int* width, unsigned int* height, float** buffer) {
    struct HdrHeader header;
    FILE* f;
    int ret = 0;

    if (!(f = fopen(filename, "rb"))) {
        fprintf(stderr, "Error: failed to open '%s'\n", filename);
    } else if (!hdr_parse_header(f, &header)) {
        fprintf(stderr, "Error: invalid HDR header '%s'\n", filename);
    } else if (header.flags & FMT_XYZ) {
        fprintf(stderr, "Error: HDR XYZE format is not supported, please use RGBE instead\n");
    } else if (!hdr_read_content(f, &header, alignRows, buffer)) {
        fprintf(stderr, "Error: invalid HDR content '%s'\n", filename);
    } else {
        *width = header.width;
        *height = header.height;
        ret = 1;
    }

    if (f) {
        fclose(f);
    }
    return ret;
}

unsigned char* hdr_to_ldr(float* hdr, unsigned int width, unsigned height, unsigned int alignRows, int alpha, float gamma, float scale) {
    float* src;
    unsigned char *ldr, *dest;
    float gammaInv = 1.0f / gamma, scaleInv = 1.0f / scale;
    unsigned int i, c, rowstrideOut, paddingIn, paddingOut, cOut = 3 + !!alpha;

    if (!width || !height || width > UINT_MAX / cOut) {
        return 0;
    }
    rowstrideOut = width * cOut;
    paddingOut = (alignRows - (rowstrideOut % alignRows)) % alignRows;
    alignRows /= sizeof(float);
    paddingIn = alignRows ? ((alignRows - ((width * 3) % alignRows)) % alignRows) : 0;

    if (rowstrideOut > UINT_MAX - paddingOut
     || (rowstrideOut += paddingOut) > UINT_MAX / height
     || !(ldr = malloc(rowstrideOut * height))) {
        return 0;
    }

    src = hdr;
    dest = ldr;
    while (height--) {
        for (i = 0; i < width; i++) {
            for (c = 0; c < 3; c++) {
                float v = (float)pow((*src++) * scaleInv, gammaInv) * 255.0f + 0.5f;
                if (v < 0.0f) {
                    v = 0.0f;
                } else if (v > 255.0f) {
                    v = 255.0f;
                }
                *dest++ = v;
            }
            if (alpha) {
                *dest++ = 255;
            }
        }
        src += paddingIn;
        dest += paddingOut;
    }

    return ldr;
}

float* ldr_to_hdr(unsigned char* ldr, unsigned int width, unsigned height, unsigned int alignRows, int alpha, float gamma, float scale) {
    float *hdr, *dest;
    unsigned char *src;
    unsigned int i, c, rowstrideOut, paddingIn, paddingOut, cIn = 3 + !!alpha;

    if (!width || !height || width > UINT_MAX / cIn) {
        return 0;
    }
    paddingIn = (alignRows - ((width * cIn) % alignRows)) % alignRows;
    alignRows /= sizeof(float);
    rowstrideOut = width * 3;
    paddingOut = alignRows ? ((alignRows - (rowstrideOut % alignRows)) % alignRows) : 0;

    if (rowstrideOut > UINT_MAX - paddingOut || height > UINT_MAX / sizeof(float)
     || (rowstrideOut += paddingOut) > UINT_MAX / (height * sizeof(float))
     || !(hdr = malloc(rowstrideOut * height * sizeof(float)))) {
        return 0;
    }

    src = ldr;
    dest = hdr;
    while (height--) {
        for (i = 0; i < width; i++) {
            for (c = 0; c < 3; c++) {
                *dest++ = ((float)pow(((float)(*src++)) / 255.0f, gamma)) * scale;
            }
            src += !!alpha;
        }
        src += paddingIn;
        dest += paddingOut;
    }

    return hdr;
}
