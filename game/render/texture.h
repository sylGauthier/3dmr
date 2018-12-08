#include <GL/glew.h>

#ifndef TEXTURE_H
#define TEXTURE_H

GLuint texture_load_from_buffer(const unsigned char* buffer, int width, int height, int alpha);
GLuint texture_load_from_file(const char* filename);

#endif
