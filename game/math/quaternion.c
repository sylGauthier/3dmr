#include <stdio.h>
#include <string.h>
#include <math.h>
#include "linear_algebra.h"
#include "quaternion.h"

void quaternion_load_id(Quaternion q) {
    q[0] = 1;
    q[1] = 0;
    q[2] = 0;
    q[3] = 0;
}

void quaternion_set_axis_angle(Quaternion q, Vec3 axis, float angle) {
    q[0] = cos(angle / 2.0);
    if (norm3(axis) > 0.0) {
        mul3sv(&q[1], sin(angle / 2.0) / norm3(axis), axis);
    } else {
        memset(&q[1], 0, sizeof(Vec3));
    }
}

void quaternion_get_axis(Quaternion q, Vec3 axis) {
    memcpy(axis, &q[1], sizeof(Vec3));
    normalize3(axis);
}

float quaternion_get_angle(Quaternion q) {
    return 2 * acos(q[0]);
}

void quaternion_set_angle(Quaternion q, float angle) {
    q[0] = cos(angle / 2.0);
    if (norm3(&q[1]) > 0.0) {
        scale3v(&q[1], sin(angle / 2.0) / norm3(&q[1]));
    }
}

void quaternion_mul(Quaternion dest, Quaternion a, Quaternion b) {
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

void quaternion_compose(Vec3 dest, Quaternion q, Vec3 v) {
    Quaternion tmp, vec, qInv;
    memcpy(qInv, q, sizeof(Quaternion));
    quaternion_inv(qInv);
    vec[0] = 0;
    memcpy(&vec[1], v, sizeof(Vec3));
    quaternion_mul(tmp, q, vec);
    quaternion_mul(vec, tmp, qInv);
    memcpy(dest, &vec[1], sizeof(Vec3));
}

void quaternion_to_mat3(Mat3 dest, Quaternion q) {
    float mag = norm3(&q[1]);
    float angle = atan2(mag, q[0]);
    if (angle > M_PI / 2.0) {
        angle -= M_PI;
        mag = -mag;
    }
    if (mag) {
        load_rot3(dest, &q[1], 2.0 * angle);
    } else {
        Vec3 axis = {1, 0, 0};
        load_rot3(dest, axis, 2.0 * angle);
    }
}

void quaternion_to_mat4(Mat4 dest, Quaternion q) {
    float mag = norm3(&q[1]);
    float angle = atan2(mag, q[0]);
    if (angle > M_PI / 2.0) {
        angle -= M_PI;
        mag = -mag;
    }
    if (mag) {
        load_rot4(dest, &q[1], 2.0 * angle);
    } else {
        Vec3 axis = {1, 0, 0};
        load_rot4(dest, axis, 2.0 * angle);
    }
}

void quaternion_from_mat3(Quaternion dest, Mat3 src) {
    Vec3 axis;
    axis[0] = src[1][2] - src[2][1];
    axis[1] = src[2][0] - src[0][2];
    axis[2] = src[0][1] - src[1][0];
    quaternion_set_axis_angle(dest, axis, atan2(norm3(axis) / 2.0, (src[0][0] + src[1][1] + src[2][2] - 1.0) / 2.0));
}

void quaternion_from_mat4(Quaternion dest, Mat4 src) {
    Vec3 axis;
    axis[0] = src[1][2] - src[2][1];
    axis[1] = src[2][0] - src[0][2];
    axis[2] = src[0][1] - src[1][0];
    quaternion_set_axis_angle(dest, axis, atan2(norm3(axis) / 2.0, (src[0][0] + src[1][1] + src[2][2] - 1.0) / 2.0));
}

void quaternion_decompose_swing_twist(Quaternion src, Vec3 direction, Quaternion swing, Quaternion twist) {
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

void quaternion_print(Quaternion q) {
    Vec3 tmp;
    memcpy(tmp, &q[1], sizeof(Vec3));
    normalize3(tmp);
    printf("Angle: %f, Axis: (%f, %f, %f)\n", 2 * acos(q[0]), tmp[0], tmp[1], tmp[2]);
}
