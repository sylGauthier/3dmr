#include <stdio.h>
#include <stdlib.h>
#include <png.h>
#include <GL/glew.h>

static int png_read(const char* filename, int* width, int* height, int* alpha, void* buffer) {
    int y, nbBytes;
    unsigned char header[8];
    FILE* in;
    int bitDepth, colorType, interlaceMethod, nPass, nbPasses;
    png_uint_32 w, h, rowStride;
    int ret = 0;
    png_structp png_ptr = NULL;
    png_infop info_ptr = NULL;
    png_byte** output = buffer;

    if ((in = fopen(filename, "rb"))) {
        nbBytes = fread(header, 1, 8, in);
        if (!png_sig_cmp(header, 0, nbBytes)) {
            if ((png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL))) {
                if ((info_ptr = png_create_info_struct(png_ptr))) {
                    if (!setjmp(png_jmpbuf(png_ptr))) {
                        png_init_io(png_ptr, in);
                        png_set_sig_bytes(png_ptr, nbBytes);

                        png_read_info(png_ptr, info_ptr);
                        png_get_IHDR(png_ptr, info_ptr, &w, &h, &bitDepth, &colorType, &interlaceMethod, NULL, NULL);
                        *alpha = (colorType == PNG_COLOR_TYPE_RGB_ALPHA) || (colorType == PNG_COLOR_TYPE_GRAY_ALPHA);
                        *width = w;
                        *height = h;

                        png_set_strip_16(png_ptr);
                        png_set_packing(png_ptr);
                        if (colorType == PNG_COLOR_TYPE_PALETTE) {
                            png_set_palette_to_rgb(png_ptr);
                        }
                        if (colorType == PNG_COLOR_TYPE_GRAY && bitDepth < 8) {
                            png_set_expand_gray_1_2_4_to_8(png_ptr);
                        }
                        if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
                            *alpha = 1;
                            png_set_tRNS_to_alpha(png_ptr);
                        }

                        nbPasses = png_set_interlace_handling(png_ptr);
                        png_read_update_info(png_ptr, info_ptr);

                        rowStride = w * ((*alpha) ? 4 : 3);
                        if ((rowStride % 4) != 0) {
                            /* OpenGL requires that the row stride is always
                             * multiple of 4 bytes. (see GL_UNPACK_ALIGNMENT)
                             */
                            rowStride += 4 - (rowStride % 4);
                        }
                        if ((*output = (png_byte*) malloc(h * rowStride))) {
                            for (nPass = 0; nPass < nbPasses; nPass++) {
                                for (y = 0; y < h; y++) {
                                    png_read_row(png_ptr, *output + y * rowStride, NULL);
                                }
                            }
                            png_read_end(png_ptr, info_ptr);
                            ret = 1;
                        } else {
                            fprintf(stderr, "Error: can't allocate memory to read %s\n", filename);
                        }
                    } else {
                        fprintf(stderr, "Error: while reading the PNG file %s \n", filename);
                    }
                    png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
                } else {
                    fprintf(stderr, "Error: could not allocate PNG info struct for %s\n", filename);
                }
                png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
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

GLuint texture_load_from_buffer(const unsigned char* buffer, int width, int height, int alpha) {
    GLuint texture = 0;

    glGenTextures(1, &texture);
    if (texture) {
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, alpha ? GL_RGBA : GL_RGB, width, height, 0, alpha ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, buffer);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }

    return texture;
}

GLuint texture_load_from_file(const char* filename) {
    GLuint texture = 0;
    unsigned char* buffer;
    int width, height, alpha;

    if (png_read(filename, &width, &height, &alpha, &buffer)) {
        texture = texture_load_from_buffer(buffer, width, height, alpha);
        free(buffer);
    }

    return texture;
}
