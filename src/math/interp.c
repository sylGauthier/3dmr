#include <math.h>
#include <3dmr/math/quaternion.h>

float lerp(float a0, float a1, float t) {
    return a0 + t * (a1 - a0);
}

float clamp(float x, float lower, float upper) {
    return (x < lower) ? lower : (x > upper) ? upper : x;
}

/* https://en.wikipedia.org/wiki/Smoothstep */
float smoothstep(float x) {
    return x * x * (3 - 2 * x);
}

float smootherstep(float x) {
    return x * x * x * (x * (x * 6 - 15) + 10);
}

void quaternion_slerp(Quaternion dest, const Quaternion q1, const Quaternion q2, float t) {
    float o = acos(dot4(q1, q2));
    float so = sin(o);
    float f1 = sin((1.0f - t) * o) / so;
    float f2 = sin(t * o) / so;
    dest[0] = f1 * q1[0] + f2 * q2[0];
    dest[1] = f1 * q1[1] + f2 * q2[1];
    dest[2] = f1 * q1[2] + f2 * q2[2];
    dest[3] = f1 * q1[3] + f2 * q2[3];
}
