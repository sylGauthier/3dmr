#include <stdio.h>
#include <string.h>
#include <math.h>
#include <3dmr/math/linear_algebra.h>
#include <3dmr/math/quaternion.h>

void quaternion_load_id(Quaternion q) {
    q[0] = 1;
    q[1] = 0;
    q[2] = 0;
    q[3] = 0;
}

void quaternion_set_axis_angle(Quaternion q, const Vec3 axis, float angle) {
    q[0] = cos(angle / 2.0);
    if (norm3(axis) > 0.0) {
        mul3sv(&q[1], sin(angle / 2.0) / norm3(axis), axis);
    } else {
        memset(&q[1], 0, sizeof(Vec3));
    }
}

void quaternion_get_axis(const Quaternion q, Vec3 axis) {
    memcpy(axis, &q[1], sizeof(Vec3));
    normalize3(axis);
}

float quaternion_get_angle(const Quaternion q) {
    return 2 * acos(q[0]);
}

void quaternion_set_angle(Quaternion q, float angle) {
    q[0] = cos(angle / 2.0);
    if (norm3(&q[1]) > 0.0) {
        scale3v(&q[1], sin(angle / 2.0) / norm3(&q[1]));
    }
}

void quaternion_mul(Quaternion dest, const Quaternion a, const Quaternion b) {
    dest[0] = a[0] * b[0] - a[1] * b[1] - a[2] * b[2] - a[3] * b[3];
    dest[1] = a[0] * b[1] + a[1] * b[0] + a[2] * b[3] - a[3] * b[2];
    dest[2] = a[0] * b[2] - a[1] * b[3] + a[2] * b[0] + a[3] * b[1];
    dest[3] = a[0] * b[3] + a[1] * b[2] - a[2] * b[1] + a[3] * b[0];
}

void quaternion_conj(Quaternion q) {
    neg3v(&q[1]);
}

void quaternion_inv(Quaternion q) {
    float nSq = norm4sq(q);
    q[0] /= nSq;
    scale3v(&q[1], -1.0 / nSq);
}

void quaternion_compose(Vec3 dest, const Quaternion q, const Vec3 v) {
    Quaternion tmp, vec, qInv;
    memcpy(qInv, q, sizeof(Quaternion));
    quaternion_inv(qInv);
    vec[0] = 0;
    memcpy(&vec[1], v, sizeof(Vec3));
    quaternion_mul(tmp, q, vec);
    quaternion_mul(vec, tmp, qInv);
    memcpy(dest, &vec[1], sizeof(Vec3));
}

void quaternion_to_mat3(Mat3 dest, const Quaternion q) {
    float a2 = q[0] * q[0], b2 = q[1] * q[1], c2 = q[2] * q[2], d2 = q[3] * q[3];
    float ab = q[0] * q[1], ac = q[0] * q[2], ad = q[0] * q[3];
    float bc = q[1] * q[2], bd = q[1] * q[3];
    float cd = q[2] * q[3];

    dest[0][0] = a2 + b2 - c2 - d2;
    dest[0][1] = 2 * bc + 2 * ad;
    dest[0][2] = 2 * bd - 2 * ac;
    dest[1][0] = 2 * bc - 2 * ad;
    dest[1][1] = a2 - b2 + c2 - d2;
    dest[1][2] = 2 * cd + 2 * ab;
    dest[2][0] = 2 * bd + 2 * ac;
    dest[2][1] = 2 * cd - 2 * ab;
    dest[2][2] = a2 - b2 - c2 + d2;
}

void quaternion_to_mat4(Mat4 dest, const Quaternion q) {
    float a2 = q[0] * q[0], b2 = q[1] * q[1], c2 = q[2] * q[2], d2 = q[3] * q[3];
    float ab = q[0] * q[1], ac = q[0] * q[2], ad = q[0] * q[3];
    float bc = q[1] * q[2], bd = q[1] * q[3];
    float cd = q[2] * q[3];

    dest[0][0] = a2 + b2 - c2 - d2;
    dest[0][1] = 2 * bc + 2 * ad;
    dest[0][2] = 2 * bd - 2 * ac;
    dest[0][3] = 0;
    dest[1][0] = 2 * bc - 2 * ad;
    dest[1][1] = a2 - b2 + c2 - d2;
    dest[1][2] = 2 * cd + 2 * ab;
    dest[1][3] = 0;
    dest[2][0] = 2 * bd + 2 * ac;
    dest[2][1] = 2 * cd - 2 * ab;
    dest[2][2] = a2 - b2 - c2 + d2;
    dest[2][3] = 0;
    zero3v(dest[3]);
    dest[3][3] = 1;
}

/* wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles#Euler_Angles_to_Quaternion_Conversion */
void quaternion_from_xyz(Quaternion dest, const Vec3 xyz) {
    float cx, cy, cz;
    float sx, sy, sz;

    cx = cos(xyz[0] / 2); cy = cos(xyz[1] / 2); cz = cos(xyz[2] / 2);
    sx = sin(xyz[0] / 2); sy = sin(xyz[1] / 2); sz = sin(xyz[2] / 2);
    dest[0] = cx * cy * cz + sx * sy * sz;
    dest[1] = sx * cy * cz - cx * sy * sz;
    dest[2] = cx * sy * cz + sx * cy * sz;
    dest[3] = cx * cy * sz - sx * sy * cz;
}

void quaternion_to_xyz(Vec3 dest, const Quaternion src) {
    float srcp, crcp, sinp, sycp, cycp;

    srcp = 2 * (src[0] * src[1] + src[2] * src[3]);
    crcp = 1 - 2 * (src[1] * src[1] + src[2] * src[2]);
    sinp = 2 * (src[0] * src[2] - src[3] * src[1]);
    sycp = 2 * (src[0] * src[3] + src[1] * src[2]);
    cycp = 1 - 2 * (src[2] * src[2] + src[3] * src[3]);

    dest[0] = atan2(srcp, crcp);
    if (fabs(sinp) > 1) dest[1] = sinp / fabs(sinp) * M_PI / 2;
    else dest[1] = asin(sinp);
    dest[2] = atan2(sycp, cycp);
}

#define quaternion_from_mat(n) \
void quaternion_from_mat##n(Quaternion dest, const Mat##n src) { \
    float S, tr = src[0][0] + src[1][1] + src[2][2]; \
    if (tr > 0) { \
        S = sqrt(tr + 1.0) * 2; \
        dest[0] = 0.25 * S; \
        dest[1] = (src[1][2] - src[2][1]) / S; \
        dest[2] = (src[2][0] - src[0][2]) / S; \
        dest[3] = (src[0][1] - src[1][0]) / S; \
    } else if ((src[0][0] > src[1][1])&(src[0][0] > src[2][2])) { \
        S = sqrt(1.0 + src[0][0] - src[1][1] - src[2][2]) * 2; \
        dest[0] = (src[1][2] - src[2][1]) / S; \
        dest[1] = 0.25 * S; \
        dest[2] = (src[1][0] + src[0][1]) / S; \
        dest[3] = (src[2][0] + src[0][2]) / S; \
    } else if (src[1][1] > src[2][2]) { \
        S = sqrt(1.0 + src[1][1] - src[0][0] - src[2][2]) * 2; \
        dest[0] = (src[2][0] - src[0][2]) / S; \
        dest[1] = (src[1][0] + src[0][1]) / S; \
        dest[2] = 0.25 * S; \
        dest[3] = (src[2][1] + src[1][2]) / S; \
    } else { \
        S = sqrt(1.0 + src[2][2] - src[0][0] - src[1][1]) * 2; \
        dest[0] = (src[0][1] - src[1][0]) / S; \
        dest[1] = (src[2][0] + src[0][2]) / S; \
        dest[2] = (src[2][1] + src[1][2]) / S; \
        dest[3] = 0.25 * S; \
    } \
}
quaternion_from_mat(3)
quaternion_from_mat(4)

void quaternion_decompose_swing_twist(const Quaternion src, const Vec3 direction, Quaternion swing, Quaternion twist) {
    Quaternion tmp;
    Vec3 d;
    float norm;

    memcpy(d, direction, sizeof(Vec3));
    normalize3(d);
    mul3sv(&twist[1], dot3(&src[1], d), d);
    twist[0] = src[0];
    if ((norm = norm4(twist)) != 0.0) {
        scale4v(twist, 1.0 / norm);
        tmp[0] = twist[0];
        tmp[1] = -twist[1];
        tmp[2] = -twist[2];
        tmp[3] = -twist[3];
        quaternion_mul(swing, src, tmp);
    } else {
        quaternion_load_id(twist);
        memcpy(swing, src, sizeof(Quaternion));
    }
}

void quaternion_print(const Quaternion q) {
    Vec3 tmp;
    memcpy(tmp, &q[1], sizeof(Vec3));
    normalize3(tmp);
    printf("Angle: %f, Axis: (%f, %f, %f)\n", 2 * acos(q[0]), tmp[0], tmp[1], tmp[2]);
}
