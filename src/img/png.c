#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <png.h>

struct PNGParam {
    FILE* in;
    char name[128];

    const char* buf;
    unsigned int bufSize;
};

static int _png_read(struct PNGParam* params, unsigned int alignRows, unsigned int* width, unsigned int* height, unsigned int* channels, unsigned int reqChannels, int vReverse, unsigned char** buffer);

int png_read_buf(const void* buf, unsigned int size, unsigned int alignRows, unsigned int* width, unsigned int* height, unsigned int* channels, unsigned int reqChannels, int vReverse, unsigned char** buffer) {
    struct PNGParam params = {0};

    params.buf = buf;
    params.bufSize = size;

    strcpy(params.name, "<gltf_buffer>");
    return _png_read(&params, alignRows, width, height, channels, reqChannels, vReverse, buffer);
}

int png_read_file(const char* filename, unsigned int alignRows, unsigned int* width, unsigned int* height, unsigned int* channels, unsigned int reqChannels, int vReverse, unsigned char** buffer) {
    struct PNGParam params = {0};

    if (!(params.in = fopen(filename, "rb"))) {
        fprintf(stderr, "Error: gltf: png: can't open file: %s\n", filename);
        return 0;
    }
    strncpy(params.name, filename, 127);
    params.name[127] = '\0';
    return _png_read(&params, alignRows, width, height, channels, reqChannels, vReverse, buffer);
}

static int _png_get_header(struct PNGParam* params, unsigned char* header) {
    if (params->in) {
        return fread(header, 1, 8, params->in);
    } else if (params->buf) {
        unsigned int m = params->bufSize >= 8 ? 8 : params->bufSize;
        memcpy(header, params->buf, m);
        params->buf += m;
        params->bufSize -= m;
        return m;
    }
    return 0;
}

static void _png_read_fn(png_structp pngStruct, png_bytep dest, size_t length) {
    struct PNGParam* params;

    params = png_get_io_ptr(pngStruct);
    if (params->bufSize < length) {
        fprintf(stderr, "Error: gltf: PNG: read error\n");
        longjmp(png_jmpbuf(pngStruct), 1);
    }
    memcpy(dest, params->buf, length);
    params->buf += length;
    params->bufSize -= length;
}

static void _png_init_io(png_structp pngStruct, struct PNGParam* params) {
    if (params->in) {
        png_init_io(pngStruct, params->in);
    } else {
        png_set_read_fn(pngStruct, params, _png_read_fn);
    }
}

static int _png_read(struct PNGParam* params, unsigned int alignRows, unsigned int* width, unsigned int* height, unsigned int* channels, unsigned int reqChannels, int vReverse, unsigned char** buffer) {
    int y, nbBytes;
    unsigned char header[8];
    int bitDepth, colorType, interlaceMethod, nPass, nbPasses;
    png_uint_32 w, h, rowStride;
    int ret = 0;
    png_structp pngStruct = NULL;
    png_infop pngInfo = NULL;
    png_byte **output = buffer, *ptr;

    if (reqChannels > 4) reqChannels = 0;

    nbBytes = _png_get_header(params, header);
    if (!png_sig_cmp(header, 0, nbBytes)) {
        if ((pngStruct = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL))) {
            if ((pngInfo = png_create_info_struct(pngStruct))) {
                if (!setjmp(png_jmpbuf(pngStruct))) {
                    _png_init_io(pngStruct, params);
                    png_set_sig_bytes(pngStruct, nbBytes);

                    png_read_info(pngStruct, pngInfo);
                    png_get_IHDR(pngStruct, pngInfo, &w, &h, &bitDepth, &colorType, &interlaceMethod, NULL, NULL);
                    *width = w;
                    *height = h;

                    png_set_strip_16(pngStruct);
                    png_set_packing(pngStruct);
                    if (colorType == PNG_COLOR_TYPE_PALETTE) {
                        png_set_palette_to_rgb(pngStruct);
                        colorType = PNG_COLOR_TYPE_RGB;
                    }
                    if (colorType == PNG_COLOR_TYPE_GRAY && bitDepth < 8) {
                        png_set_expand_gray_1_2_4_to_8(pngStruct);
                    }
                    if ((colorType == PNG_COLOR_TYPE_GRAY || colorType == PNG_COLOR_TYPE_GRAY_ALPHA) && reqChannels >= 3) {
                        png_set_gray_to_rgb(pngStruct);
                        colorType |= PNG_COLOR_MASK_COLOR;
                    }
                    if ((colorType == PNG_COLOR_TYPE_RGB || colorType == PNG_COLOR_TYPE_RGB_ALPHA) && reqChannels && reqChannels < 3) {
                        png_set_rgb_to_gray_fixed(pngStruct, 1, -1, -1);
                        colorType &= ~PNG_COLOR_MASK_COLOR;
                    }
                    if (png_get_valid(pngStruct, pngInfo, PNG_INFO_tRNS)) {
                        png_set_tRNS_to_alpha(pngStruct);
                        colorType |= PNG_COLOR_MASK_ALPHA;
                    }
                    if ((colorType == PNG_COLOR_TYPE_GRAY_ALPHA && reqChannels == 1)
                            || (colorType == PNG_COLOR_TYPE_RGB_ALPHA && reqChannels == 3)) {
                        png_set_strip_alpha(pngStruct);
                        colorType &= ~PNG_COLOR_MASK_ALPHA;
                    }
                    if ((colorType == PNG_COLOR_TYPE_GRAY && (reqChannels == 2 || reqChannels == 4))
                            || (colorType == PNG_COLOR_TYPE_RGB && reqChannels == 4)) {
                        png_set_filler(pngStruct, 0xFF, PNG_FILLER_AFTER);
                        colorType |= PNG_COLOR_MASK_ALPHA;
                    }
                    switch (colorType) {
                        case PNG_COLOR_TYPE_GRAY: *channels = 1; break;
                        case PNG_COLOR_TYPE_GRAY_ALPHA: *channels = 2; break;
                        case PNG_COLOR_TYPE_RGB: *channels = 3; break;
                        case PNG_COLOR_TYPE_RGB_ALPHA: *channels = 4; break;
                        default: longjmp(png_jmpbuf(pngStruct), 1);
                    }

                    nbPasses = png_set_interlace_handling(pngStruct);
                    png_read_update_info(pngStruct, pngInfo);

                    rowStride = w * (*channels);
                    if (alignRows && (rowStride % alignRows) != 0) {
                        rowStride += alignRows - (rowStride % alignRows);
                    }
                    if ((*output = (png_byte*)malloc(h * rowStride))) {
                        for (nPass = 0; nPass < nbPasses; nPass++) {
                            if (vReverse) {
                                ptr = *output + (h - 1) * rowStride;
                                for (y = 0; y < h; y++) {
                                    png_read_row(pngStruct, ptr, NULL);
                                    ptr -= rowStride;
                                }
                            } else {
                                ptr = *output;
                                for (y = 0; y < h; y++) {
                                    png_read_row(pngStruct, ptr, NULL);
                                    ptr += rowStride;
                                }
                            }
                        }
                        png_read_end(pngStruct, pngInfo);
                        ret = 1;
                    } else {
                        fprintf(stderr, "Error: can't allocate memory to read %s\n", params->name);
                    }
                } else {
                    fprintf(stderr, "Error: while reading the PNG file %s \n", params->name);
                }
                png_free_data(pngStruct, pngInfo, PNG_FREE_ALL, -1);
            } else {
                fprintf(stderr, "Error: could not allocate PNG info struct for %s\n", params->name);
            }
            png_destroy_read_struct(&pngStruct, &pngInfo, NULL);
        } else {
            fprintf(stderr, "Error: could not allocate PNG read struct for %s\n", params->name);
        }
    } else {
        fprintf(stderr, "Error: %s is not a PNG file\n", params->name);
    }

    if (params->in) fclose(params->in);
    return ret;
}

int png_write(const char* filename, unsigned int alignRows, unsigned int width, unsigned int height, unsigned int channels, int vReverse, const unsigned char* buffer) {
    int y;
    FILE* out;
    const unsigned char* ptr = buffer;
    int ret = 0;
    png_structp pngStruct = NULL;
    png_infop pngInfo = NULL;
    unsigned int rowStride = channels * width, format;

    switch (channels) {
        case 1: format = PNG_COLOR_TYPE_GRAY; break;
        case 2: format = PNG_COLOR_TYPE_GRAY_ALPHA; break;
        case 3: format = PNG_COLOR_TYPE_RGB; break;
        case 4: format = PNG_COLOR_TYPE_RGB_ALPHA; break;
        default: return 0;
    }
    if (alignRows && (rowStride % alignRows) != 0) {
        rowStride += alignRows - (rowStride % alignRows);
    }
    if ((out = fopen(filename, "wb"))) {
        if ((pngStruct = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL))) {
            if ((pngInfo = png_create_info_struct(pngStruct))) {
                if (!setjmp(png_jmpbuf(pngStruct))) {
                    png_init_io(pngStruct, out);
                    png_set_IHDR(pngStruct, pngInfo, width, height, 8, format, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

                    png_write_info(pngStruct, pngInfo);
                    if (vReverse) {
                        ptr += (height - 1) * rowStride;
                        for (y = 0; y < height; y++) {
                            png_write_row(pngStruct, ptr);
                            ptr -= rowStride;
                        }
                    } else {
                        for (y = 0; y < height; y++) {
                            png_write_row(pngStruct, ptr);
                            ptr += rowStride;
                        }
                    }
                    png_write_end(pngStruct, NULL);

                    ret = 1;
                } else {
                    fprintf(stderr, "Error: during png creation, %s\n", filename);
                }
                png_free_data(pngStruct, pngInfo, PNG_FREE_ALL, -1);
            } else {
                fprintf(stderr, "Error: could not allocate info struct for %s\n", filename);
            }
            png_destroy_write_struct(&pngStruct, &pngInfo);
        } else {
            fprintf(stderr, "Error: could not allocate write struct for %s\n", filename);
        }
        fclose(out);
    } else {
        fprintf(stderr, "Error: could not open file %s for writing\n", filename);
    }

    return ret;
}
