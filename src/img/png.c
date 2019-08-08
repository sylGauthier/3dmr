#include <stdio.h>
#include <stdlib.h>
#include <png.h>

int png_read(const char* filename, unsigned int alignRows, unsigned int* width, unsigned int* height, unsigned int* channels, unsigned int reqChannels, int vReverse, unsigned char** buffer) {
    int y, nbBytes;
    unsigned char header[8];
    FILE* in;
    int bitDepth, colorType, interlaceMethod, nPass, nbPasses;
    png_uint_32 w, h, rowStride;
    int ret = 0;
    png_structp pngStruct = NULL;
    png_infop pngInfo = NULL;
    png_byte **output = buffer, *ptr;

    if (reqChannels > 4) reqChannels = 0;

    if ((in = fopen(filename, "rb"))) {
        nbBytes = fread(header, 1, 8, in);
        if (!png_sig_cmp(header, 0, nbBytes)) {
            if ((pngStruct = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL))) {
                if ((pngInfo = png_create_info_struct(pngStruct))) {
                    if (!setjmp(png_jmpbuf(pngStruct))) {
                        png_init_io(pngStruct, in);
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
                            fprintf(stderr, "Error: can't allocate memory to read %s\n", filename);
                        }
                    } else {
                        fprintf(stderr, "Error: while reading the PNG file %s \n", filename);
                    }
                    png_free_data(pngStruct, pngInfo, PNG_FREE_ALL, -1);
                } else {
                    fprintf(stderr, "Error: could not allocate PNG info struct for %s\n", filename);
                }
                png_destroy_read_struct(&pngStruct, &pngInfo, NULL);
            } else {
                fprintf(stderr, "Error: could not allocate PNG read struct for %s\n", filename);
            }
        } else {
            fprintf(stderr, "Error: %s is not a PNG file\n", filename);
        }
        fclose(in);
    } else {
        fprintf(stderr, "Error: could not open file %s for reading\n", filename);
    }

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
