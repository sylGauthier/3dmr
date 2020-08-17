#include <math.h>
#include <string.h>
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

/* https://en.wikipedia.org/wiki/Slerp */
void quaternion_slerp(Quaternion dest, const Quaternion q1, const Quaternion q2, float t) {
    Quaternion qr;
    float dotProd = dot4(q1, q2);

    memcpy(qr, q2, sizeof(Quaternion));

    if (dotProd < 0.) {
        scale4v(qr, -1);
        dotProd = -dotProd;
    }

    /* If quaternions are too close for acos, linear interp */
    if (dotProd > 0.9995) {
        dest[0] = q1[0] + t * (qr[0] - q1[0]);
        dest[1] = q1[1] + t * (qr[1] - q1[1]);
        dest[2] = q1[2] + t * (qr[2] - q1[2]);
        dest[3] = q1[3] + t * (qr[3] - q1[3]);
        normalize4(dest);
    } else {
        float o = acos(dotProd);
        float so = sin(o);
        float f1 = sin((1.0f - t) * o) / so;
        float f2 = sin(t * o) / so;
        dest[0] = f1 * q1[0] + f2 * qr[0];
        dest[1] = f1 * q1[1] + f2 * qr[1];
        dest[2] = f1 * q1[2] + f2 * qr[2];
        dest[3] = f1 * q1[3] + f2 * qr[3];
    }
}
