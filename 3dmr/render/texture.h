#include <GL/glew.h>

#ifndef TDMR_RENDER_TEXTURE_H
#define TDMR_RENDER_TEXTURE_H

GLuint texture_load_from_uchar_buffer(const unsigned char* buffer, unsigned int width, unsigned int height, unsigned int channels, unsigned int ralign);
GLuint texture_load_from_float_buffer(const float* buffer, unsigned int width, unsigned int height, unsigned int channels, unsigned int ralign);
GLuint texture_load_from_png(const char* filename);

#endif
