#include <GL/glew.h>
#include <game/math/linear_algebra.h>

#ifndef CHECKERBOARD_H
#define CHECKERBOARD_H

GLuint default_checkerboard();

GLuint texture_checkerboard(Vec4 color1, Vec4 color2);

#endif
