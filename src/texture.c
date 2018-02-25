#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include "png.h"

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
    unsigned int width, height;
    int alpha;

    if (png_read(filename, 4, &width, &height, &alpha, &buffer)) {
        texture = texture_load_from_buffer(buffer, width, height, alpha);
        free(buffer);
    }

    return texture;
}
