#include <game/math/quaternion.h>

#ifndef GAME_MATH_INTERP_H
#define GAME_MATH_INTERP_H

float lerp(float a0, float a1, float t);

float clamp(float x, float lower, float upper);

/* https://en.wikipedia.org/wiki/Smoothstep */
float smoothstep(float x);
float smootherstep(float x);

void quaternion_slerp(Quaternion dest, const Quaternion q1, const Quaternion q2, float t);

#endif
