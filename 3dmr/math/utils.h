#ifndef TDMR_MATH_UTILS_H
#define TDMR_MATH_UTILS_H

#include <3dmr/math/linear_algebra.h>

/* Will tamper src */
int mat4toposrotscale(RESTRICT_MAT4(src), RESTRICT_VEC3(pos), RESTRICT_VEC4(rot), RESTRICT_VEC3(scale));

#endif
