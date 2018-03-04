#ifndef CHECKERBOARD_H
#define CHECKERBOARD_H
#include <GL/glew.h>
#include "linear_algebra.h"
#include "texture.h"

GLuint default_checkerboard();

GLuint texture_checkerboard(Vec4 color1, Vec4 color2);

#endif
