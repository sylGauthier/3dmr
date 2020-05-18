#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <3dmr/img/png.h>

static GLenum format_type(unsigned int channels) {
    switch (channels) {
        case 1: return GL_RED;
        case 2: return GL_RG;
        case 3: return GL_RGB;
        case 4: return GL_RGBA;
    }
    return GL_NONE;
}

static GLuint texture_load_from_buffer(const void* buffer, unsigned int width, unsigned int height, unsigned int channels, unsigned int ralign, GLenum type) {
    GLuint texture = 0;
    GLenum format;

    if ((format = format_type(channels)) == GL_NONE) return 0;

    glGenTextures(1, &texture);
    if (texture) {
        glBindTexture(GL_TEXTURE_2D, texture);
        if (ralign) glPixelStorei(GL_UNPACK_ALIGNMENT, ralign);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, type, buffer);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }

    return texture;
}

GLuint texture_load_from_uchar_buffer(const unsigned char* buffer, unsigned int width, unsigned int height, unsigned int channels, unsigned int ralign) {
    return texture_load_from_buffer(buffer, width, height, channels, ralign, GL_UNSIGNED_BYTE);
}

GLuint texture_load_from_float_buffer(const float* buffer, unsigned int width, unsigned int height, unsigned int channels, unsigned int ralign) {
    return texture_load_from_buffer(buffer, width, height, channels, ralign, GL_FLOAT);
}

GLuint texture_load_from_png(const char* filename) {
    GLuint texture = 0;
    unsigned char* buffer;
    unsigned int width, height, channels;
    GLint ralign = 0;

    glGetIntegerv(GL_UNPACK_ALIGNMENT, &ralign);

    if (png_read(filename, ralign, &width, &height, &channels, 0, 1, &buffer)) {
        texture = texture_load_from_uchar_buffer(buffer, width, height, channels, 0);
        free(buffer);
    }

    return texture;
}
