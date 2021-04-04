#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <jpeglib.h>

static jmp_buf env;

static int _jpeg_do_read(struct jpeg_decompress_struct* cinfo,
                         unsigned int     alignRows,
                         unsigned int*    width,
                         unsigned int*    height,
                         unsigned int*    channels,
                         unsigned int     reqChannels,
                         int              vReverse,
                         unsigned char**  buffer) {
    J_COLOR_SPACE colorSpace;
    unsigned int rowStride;

    jpeg_read_header(cinfo, 1);

    if (reqChannels > 4) reqChannels = 0;
    if (reqChannels) {
        switch (reqChannels) {
            case 1:
                colorSpace = JCS_GRAYSCALE;
                break;
            case 2:
                fprintf(stderr, "Error: jpeg: gray/alpha not supported (reqChannels == 2)\n");
                longjmp(env, 1);
            case 3:
                colorSpace = JCS_RGB;
                break;
            case 4:
                colorSpace = JCS_EXT_RGBA;
                break;
            default:
                longjmp(env, 1);
        }
    } else {
        switch (cinfo->jpeg_color_space) {
            case JCS_UNKNOWN:
                longjmp(env, 1);
            case JCS_GRAYSCALE:
                colorSpace = JCS_GRAYSCALE;
                break;
            case JCS_RGB:
            case JCS_YCbCr:
            case JCS_YCCK:
            case JCS_CMYK:
            case JCS_EXT_RGB:
            case JCS_EXT_RGBX:
            case JCS_EXT_BGR:
            case JCS_EXT_BGRX:
            case JCS_EXT_XBGR:
            case JCS_EXT_XRGB:
                colorSpace = JCS_RGB;
                break;
            case JCS_EXT_RGBA:
            case JCS_EXT_BGRA:
            case JCS_EXT_ABGR:
            case JCS_EXT_ARGB:
            case JCS_RGB565:
                colorSpace = JCS_EXT_RGBA;
                break;
            default:
                longjmp(env, 1);

        }
    }
    cinfo->out_color_space = colorSpace;

    jpeg_start_decompress(cinfo);

    *width = cinfo->output_width;
    *height = cinfo->output_height;
    *channels = cinfo->output_components;

    rowStride = *width * (*channels);
    if (alignRows && (rowStride % alignRows) != 0) {
        rowStride += alignRows - (rowStride % alignRows);
    }

    if (!(*buffer = malloc(rowStride * *height))) {
        fprintf(stderr, "Error: jpeg: can't allocate buffer\n");
        longjmp(env, 1);
    }
    if (vReverse) {
        unsigned int i;
        unsigned char* ptr = NULL;

        ptr = *buffer + (*height - 1) * rowStride;
        for (i = 0; i < *height;) {
            i += jpeg_read_scanlines(cinfo, &ptr, 1);
            ptr -= rowStride;
        }
    } else {
        unsigned int i;
        unsigned char* ptr = NULL;

        ptr = *buffer;
        for (i = 0; i < *height;) {
            i += jpeg_read_scanlines(cinfo, &ptr, 1);
            ptr += rowStride;
        }
    }
    jpeg_finish_decompress(cinfo);
    return 1;
}

static void _error_exit(j_common_ptr cinfo) {
    longjmp(env, 1);
}

int jpeg_read_file(const char*      filename,
                   unsigned int     alignRows,
                   unsigned int*    width,
                   unsigned int*    height,
                   unsigned int*    channels,
                   unsigned int     reqChannels,
                   int              vReverse,
                   unsigned char**  buffer) {
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    FILE* f = NULL;
    char ok = 0;

    if (!(f = fopen(filename, "rb"))) {
        fprintf(stderr, "Error: jpeg: can't open file %s\n", filename);
        return 0;
    }
    cinfo.err = jpeg_std_error(&jerr);
    jerr.error_exit = _error_exit;

    if (!setjmp(env)) {
        jpeg_create_decompress(&cinfo);
        jpeg_stdio_src(&cinfo, f);

        ok = _jpeg_do_read(&cinfo, alignRows, width, height, channels, reqChannels, vReverse, buffer);
    } else {
        ok = 0;
    }
    jpeg_destroy_decompress(&cinfo);
    return ok;
}

int jpeg_read_buf(const void*       buf,
                  unsigned int      size,
                  unsigned int      alignRows,
                  unsigned int*     width,
                  unsigned int*     height,
                  unsigned int*     channels,
                  unsigned int      reqChannels,
                  int               vReverse,
                  unsigned char**   buffer) {
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    char ok = 0;

    cinfo.err = jpeg_std_error(&jerr);
    jerr.error_exit = _error_exit;

    if (!setjmp(env)) {
        jpeg_create_decompress(&cinfo);
        jpeg_mem_src(&cinfo, buf, size);

        ok = _jpeg_do_read(&cinfo, alignRows, width, height, channels, reqChannels, vReverse, buffer);
    } else {
        ok = 0;
    }
    jpeg_destroy_decompress(&cinfo);
    return ok;
}
