#include <GL/glew.h>

#ifndef TEXTURE_H
#define TEXTURE_H

GLuint texture_load_from_uchar_buffer(const unsigned char* buffer, unsigned int width, unsigned int height, unsigned int channels, unsigned int ralign);
GLuint texture_load_from_float_buffer(const float* buffer, unsigned int width, unsigned int height, unsigned int channels, unsigned int ralign);
GLuint texture_load_from_png(const char* filename);

#endif
