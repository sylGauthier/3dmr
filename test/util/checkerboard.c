#include "checkerboard.h"

GLuint default_checkerboard() {
    Vec4 black =  {0,0,0, 1};
    Vec4 purple = {1,0,1, 1};

    return texture_checkerboard(black, purple);
}

GLuint texture_checkerboard(Vec4 color1, Vec4 color2) {
    unsigned char buffer[4*4];
    int i = 0;

    buffer[i++] = 255 * color1[0];
    buffer[i++] = 255 * color1[1];
    buffer[i++] = 255 * color1[2];
    buffer[i++] = 255 * color1[3];

    buffer[i++] = 255 * color2[0];
    buffer[i++] = 255 * color2[1];
    buffer[i++] = 255 * color2[2];
    buffer[i++] = 255 * color2[3];

    buffer[i++] = 255 * color2[0];
    buffer[i++] = 255 * color2[1];
    buffer[i++] = 255 * color2[2];
    buffer[i++] = 255 * color2[3];

    buffer[i++] = 255 * color1[0];
    buffer[i++] = 255 * color1[1];
    buffer[i++] = 255 * color1[2];
    buffer[i++] = 255 * color1[3];

    return texture_load_from_buffer(buffer, 2, 2, 1);
}
