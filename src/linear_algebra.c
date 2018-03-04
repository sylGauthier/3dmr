#include <stdio.h>
#include <string.h>
#include <math.h>
#include "linear_algebra.h"

const Vec3 VEC3_AXIS_X = {1,0,0};
const Vec3 VEC3_AXIS_Y = {0,1,0};
const Vec3 VEC3_AXIS_Z = {0,0,1};

#define SWAP(a, b) tmp = a; a = b; b = tmp;
#define VAL "%8.2f"
#define SEP ", "

void normalize2(Vec2 v) {
    float norm = sqrt(v[0] * v[0] + v[1] * v[1]);
    v[0] /= norm;
    v[1] /= norm;
}

void mul2sv(Vec2 dest, float s, Vec2 v) {
    dest[0] = s * v[0];
    dest[1] = s * v[1];
}

float dot2(Vec2 v1, Vec2 v2) {
    return v1[0] * v2[0] + v1[1] * v2[1];
}

void add3v(RESTRICT_VEC3(dest), RESTRICT_VEC3(u), RESTRICT_VEC3(v)) {
    dest[0] = u[0] + v[0];
    dest[1] = u[1] + v[1];
    dest[2] = u[2] + v[2];
}

void incr3v(RESTRICT_VEC3(dest), RESTRICT_VEC3(incr)) {
    dest[0] += incr[0];
    dest[1] += incr[1];
    dest[2] += incr[2];
}

void sub3v(RESTRICT_VEC3(dest), RESTRICT_VEC3(u), RESTRICT_VEC3(v)) {
    dest[0] = u[0] - v[0];
    dest[1] = u[1] - v[1];
    dest[2] = u[2] - v[2];
}

void decr3v(RESTRICT_VEC3(dest), RESTRICT_VEC3(decr)) {
    dest[0] -= decr[0];
    dest[1] -= decr[1];
    dest[2] -= decr[2];
}

void mul3sv(RESTRICT_VEC3(dest), float s, RESTRICT_VEC3(v)) {
    dest[0] = s * v[0];
    dest[1] = s * v[1];
    dest[2] = s * v[2];
}

void scale3v(Vec3 dest, float s) {
    dest[0] *= s;
    dest[1] *= s;
    dest[2] *= s;
}

void mul3mv(RESTRICT_VEC3(dest), RESTRICT_MAT3(m), RESTRICT_VEC3(v)) {
    dest[0] = m[0][0] * v[0] + m[1][0] * v[1] + m[2][0] * v[2];
    dest[1] = m[0][1] * v[0] + m[1][1] * v[1] + m[2][1] * v[2];
    dest[2] = m[0][2] * v[0] + m[1][2] * v[1] + m[2][2] * v[2];
}

void mul3mm(RESTRICT_MAT3(dest), RESTRICT_MAT3(a), RESTRICT_MAT3(b)) {
    dest[0][0] = a[0][0] * b[0][0] + a[1][0] * b[0][1] + a[2][0] * b[0][2];
    dest[0][1] = a[0][1] * b[0][0] + a[1][1] * b[0][1] + a[2][1] * b[0][2];
    dest[0][2] = a[0][2] * b[0][0] + a[1][2] * b[0][1] + a[2][2] * b[0][2];
    dest[1][0] = a[0][0] * b[1][0] + a[1][0] * b[1][1] + a[2][0] * b[1][2];
    dest[1][1] = a[0][1] * b[1][0] + a[1][1] * b[1][1] + a[2][1] * b[1][2];
    dest[1][2] = a[0][2] * b[1][0] + a[1][2] * b[1][1] + a[2][2] * b[1][2];
    dest[2][0] = a[0][0] * b[2][0] + a[1][0] * b[2][1] + a[2][0] * b[2][2];
    dest[2][1] = a[0][1] * b[2][0] + a[1][1] * b[2][1] + a[2][1] * b[2][2];
    dest[2][2] = a[0][2] * b[2][0] + a[1][2] * b[2][1] + a[2][2] * b[2][2];
}

void neg3v(Vec3 v) {
    v[0] = -v[0];
    v[1] = -v[1];
    v[2] = -v[2];
}

void neg3m(Mat3 m) {
    m[0][0] = -m[0][0];
    m[0][1] = -m[0][1];
    m[0][2] = -m[0][2];
    m[1][0] = -m[1][0];
    m[1][1] = -m[1][1];
    m[1][2] = -m[1][2];
    m[2][0] = -m[2][0];
    m[2][1] = -m[2][1];
    m[2][2] = -m[2][2];
}

void transpose3m(Mat3 m) {
    float tmp;
    SWAP(m[0][1], m[1][0]);
    SWAP(m[0][2], m[2][0]);
    SWAP(m[1][2], m[2][1]);
}

float det3(Mat3 m) {
    return m[0][0] * m[1][1] * m[2][2] + m[0][1] * m[1][2] * m[2][0] + m[1][0] * m[2][1] * m[0][2]
         - m[1][1] * m[0][2] * m[2][0] - m[0][0] * m[1][2] * m[2][1] - m[0][1] * m[1][0] * m[2][2];
}

int invert3m(RESTRICT_MAT3(dest), RESTRICT_MAT3(src)) {
    float det = det3(src);

    if (det != 0) {
        float oneOverDet = 1.0 / det;

        dest[0][0] = oneOverDet * (src[1][1] * src[2][2] - src[2][1] * src[1][2]);
        dest[0][1] = oneOverDet * (src[2][1] * src[0][2] - src[0][1] * src[2][2]);
        dest[0][2] = oneOverDet * (src[0][1] * src[1][2] - src[1][1] * src[0][2]);

        dest[1][0] = oneOverDet * (src[2][0] * src[1][2] - src[1][0] * src[2][2]);
        dest[1][1] = oneOverDet * (src[0][0] * src[2][2] - src[2][0] * src[0][2]);
        dest[1][2] = oneOverDet * (src[1][0] * src[0][2] - src[0][0] * src[1][2]);

        dest[2][0] = oneOverDet * (src[1][0] * src[2][1] - src[1][1] * src[2][0]);
        dest[2][1] = oneOverDet * (src[2][0] * src[0][1] - src[0][0] * src[2][1]);
        dest[2][2] = oneOverDet * (src[0][0] * src[1][1] - src[1][0] * src[0][1]);

        return 1;
    }

    return 0;
}

float norm3sq(Vec3 v) {
    return v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
}

float norm3(Vec3 v) {
    return sqrt(norm3sq(v));
}

void normalize3(Vec3 v) {
    float n = norm3(v);
    v[0] /= n;
    v[1] /= n;
    v[2] /= n;
}

float dot3(Vec3 u, Vec3 v) {
    return u[0] * v[0] + u[1] * v[1] + u[2] * v[2];
}

void cross3(Vec3 dest, Vec3 u, Vec3 v) {
    dest[0] = u[1] * v[2] - u[2] * v[1];
    dest[1] = u[2] * v[0] - u[0] * v[2];
    dest[2] = u[0] * v[1] - u[1] * v[0];
}

void row3m3(RESTRICT_VEC3(dest), RESTRICT_MAT3(m), unsigned int row) {
    dest[0] = m[0][row];
    dest[1] = m[1][row];
    dest[2] = m[2][row];
}

void row3m4(RESTRICT_VEC3(dest), RESTRICT_MAT4(m), unsigned int row) {
    dest[0] = m[0][row];
    dest[1] = m[1][row];
    dest[2] = m[2][row];
}

void load_id3(Mat3 dest) {
    memset(dest, 0, sizeof(Mat3));
    dest[0][0] = 1;
    dest[1][1] = 1;
    dest[2][2] = 1;
}

void print3v(Vec3 v) {
    printf("/" VAL " \\\n|" VAL " |\n\\" VAL " /\n", v[0], v[1], v[2]);
}

void print3m(Mat3 m) {
    printf(
            "/"  VAL SEP VAL SEP VAL " \\\n"
            "|"  VAL SEP VAL SEP VAL " |\n"
            "\\" VAL SEP VAL SEP VAL " /\n",
            m[0][0], m[1][0], m[2][0],
            m[0][1], m[1][1], m[2][1],
            m[0][2], m[1][2], m[2][2]
          );
}

void vec4to3(RESTRICT_VEC3(dest), RESTRICT_VEC3(src)) {
    memcpy(dest, src, sizeof(Vec3));
}

void mat4to3(RESTRICT_MAT3(dest), RESTRICT_MAT4(src)) {
    memcpy(dest[0], src[0], sizeof(Vec3));
    memcpy(dest[1], src[1], sizeof(Vec3));
    memcpy(dest[2], src[2], sizeof(Vec3));
}

void mat3to4(RESTRICT_MAT4(dest), RESTRICT_MAT3(src)) {
    memcpy(dest[0], src[0], sizeof(Vec3));
    memcpy(dest[1], src[1], sizeof(Vec3));
    memcpy(dest[2], src[2], sizeof(Vec3));
}

void add4v(RESTRICT_VEC4(dest), RESTRICT_VEC4(u), RESTRICT_VEC4(v)) {
    dest[0] = u[0] + v[0];
    dest[1] = u[1] + v[1];
    dest[2] = u[2] + v[2];
    dest[3] = u[3] + v[3];
}

void mul4sv(RESTRICT_VEC4(dest), float s, RESTRICT_VEC4(v)) {
    dest[0] = s * v[0];
    dest[1] = s * v[1];
    dest[2] = s * v[2];
    dest[3] = s * v[3];
}

void mul4sm(RESTRICT_MAT4(dest), float s, RESTRICT_MAT4(m)) {
    dest[0][0] = m[0][0] * s;
    dest[0][1] = m[0][1] * s;
    dest[0][2] = m[0][2] * s;
    dest[0][3] = m[0][3] * s;
    dest[1][0] = m[1][0] * s;
    dest[1][1] = m[1][1] * s;
    dest[1][2] = m[1][2] * s;
    dest[1][3] = m[1][3] * s;
    dest[2][0] = m[2][0] * s;
    dest[2][1] = m[2][1] * s;
    dest[2][2] = m[2][2] * s;
    dest[2][3] = m[2][3] * s;
    dest[3][0] = m[3][0] * s;
    dest[3][1] = m[3][1] * s;
    dest[3][2] = m[3][2] * s;
    dest[3][3] = m[3][3] * s;
}

void scale4v(Vec4 dest, float s) {
    dest[0] *= s;
    dest[1] *= s;
    dest[2] *= s;
    dest[3] *= s;
}

void mul4mv(RESTRICT_VEC4(res), RESTRICT_MAT4(m), RESTRICT_VEC4(v)) {
    res[0] = m[0][0] * v[0] + m[1][0] * v[1] + m[2][0] * v[2] + m[3][0] * v[3];
    res[1] = m[0][1] * v[0] + m[1][1] * v[1] + m[2][1] * v[2] + m[3][1] * v[3];
    res[2] = m[0][2] * v[0] + m[1][2] * v[1] + m[2][2] * v[2] + m[3][2] * v[3];
    res[3] = m[0][3] * v[0] + m[1][3] * v[1] + m[2][3] * v[2] + m[3][3] * v[3];
}

void mul4mm(RESTRICT_MAT4(res), RESTRICT_MAT4(a), RESTRICT_MAT4(b)) {
    res[0][0] = a[0][0] * b[0][0] + a[1][0] * b[0][1] + a[2][0] * b[0][2] + a[3][0] * b[0][3];
    res[0][1] = a[0][1] * b[0][0] + a[1][1] * b[0][1] + a[2][1] * b[0][2] + a[3][1] * b[0][3];
    res[0][2] = a[0][2] * b[0][0] + a[1][2] * b[0][1] + a[2][2] * b[0][2] + a[3][2] * b[0][3];
    res[0][3] = a[0][3] * b[0][0] + a[1][3] * b[0][1] + a[2][3] * b[0][2] + a[3][3] * b[0][3];
    res[1][0] = a[0][0] * b[1][0] + a[1][0] * b[1][1] + a[2][0] * b[1][2] + a[3][0] * b[1][3];
    res[1][1] = a[0][1] * b[1][0] + a[1][1] * b[1][1] + a[2][1] * b[1][2] + a[3][1] * b[1][3];
    res[1][2] = a[0][2] * b[1][0] + a[1][2] * b[1][1] + a[2][2] * b[1][2] + a[3][2] * b[1][3];
    res[1][3] = a[0][3] * b[1][0] + a[1][3] * b[1][1] + a[2][3] * b[1][2] + a[3][3] * b[1][3];
    res[2][0] = a[0][0] * b[2][0] + a[1][0] * b[2][1] + a[2][0] * b[2][2] + a[3][0] * b[2][3];
    res[2][1] = a[0][1] * b[2][0] + a[1][1] * b[2][1] + a[2][1] * b[2][2] + a[3][1] * b[2][3];
    res[2][2] = a[0][2] * b[2][0] + a[1][2] * b[2][1] + a[2][2] * b[2][2] + a[3][2] * b[2][3];
    res[2][3] = a[0][3] * b[2][0] + a[1][3] * b[2][1] + a[2][3] * b[2][2] + a[3][3] * b[2][3];
    res[3][0] = a[0][0] * b[3][0] + a[1][0] * b[3][1] + a[2][0] * b[3][2] + a[3][0] * b[3][3];
    res[3][1] = a[0][1] * b[3][0] + a[1][1] * b[3][1] + a[2][1] * b[3][2] + a[3][1] * b[3][3];
    res[3][2] = a[0][2] * b[3][0] + a[1][2] * b[3][1] + a[2][2] * b[3][2] + a[3][2] * b[3][3];
    res[3][3] = a[0][3] * b[3][0] + a[1][3] * b[3][1] + a[2][3] * b[3][2] + a[3][3] * b[3][3];
}

void neg4v(Vec4 v) {
    v[0] = -v[0];
    v[1] = -v[1];
    v[2] = -v[2];
    v[3] = -v[3];
}

void neg4m(Mat4 m) {
    m[0][0] = -m[0][0];
    m[0][1] = -m[0][1];
    m[0][2] = -m[0][2];
    m[0][3] = -m[0][3];
    m[1][0] = -m[1][0];
    m[1][1] = -m[1][1];
    m[1][2] = -m[1][2];
    m[1][3] = -m[1][3];
    m[2][0] = -m[2][0];
    m[2][1] = -m[2][1];
    m[2][2] = -m[2][2];
    m[2][3] = -m[2][3];
    m[3][0] = -m[3][0];
    m[3][1] = -m[3][1];
    m[3][2] = -m[3][2];
    m[3][3] = -m[3][3];
}

void transpose4m(Mat4 m) {
    float tmp;
    SWAP(m[0][1], m[1][0]);
    SWAP(m[0][2], m[2][0]);
    SWAP(m[0][3], m[3][0]);
    SWAP(m[1][2], m[2][1]);
    SWAP(m[1][3], m[3][1]);
    SWAP(m[2][3], m[3][2]);
}

float norm4sq(Vec4 v) {
    return v[0] * v[0] + v[1] * v[1] + v[2] * v[2] + v[3] * v[3];
}

float norm4(Vec4 v) {
    return sqrt(norm4sq(v));
}

void normalize4(Vec4 v) {
    float n = norm4(v);
    v[0] /= n;
    v[1] /= n;
    v[2] /= n;
    v[3] /= n;
}

float dot4(Vec4 u, Vec4 v) {
    return u[0] * v[0] + u[1] * v[1] + u[2] * v[2] + u[3] * v[3];
}

void row4(RESTRICT_VEC4(dest), RESTRICT_MAT4(m), unsigned int row) {
    dest[0] = m[0][row];
    dest[1] = m[1][row];
    dest[2] = m[2][row];
    dest[3] = m[3][row];
}

void load_id4(Mat4 dest) {
    memset(dest, 0, sizeof(Mat4));
    dest[0][0] = 1;
    dest[1][1] = 1;
    dest[2][2] = 1;
    dest[3][3] = 1;
}

void print4v(Vec4 v) {
    printf("/" VAL " \\\n|" VAL " |\n|" VAL " |\n\\" VAL " /\n", v[0], v[1], v[2], v[3]);
}

void print4m(Mat4 m) {
    printf(
            "/"  VAL SEP VAL SEP VAL SEP VAL " \\\n"
            "|"  VAL SEP VAL SEP VAL SEP VAL " |\n"
            "|"  VAL SEP VAL SEP VAL SEP VAL " |\n"
            "\\" VAL SEP VAL SEP VAL SEP VAL " /\n",
            m[0][0], m[1][0], m[2][0], m[3][0],
            m[0][1], m[1][1], m[2][1], m[3][1],
            m[0][2], m[1][2], m[2][2], m[3][2],
            m[0][3], m[1][3], m[2][3], m[3][3]
          );
}

void load_rot3(RESTRICT_MAT3(dest), RESTRICT_VEC3(axis), float angle) {
    Vec3 a;
    float s = sin(angle), c = cos(angle);
    float xv, xs, yv, ys, zv, zs, ver = 1 - c;

    memcpy(a, axis, sizeof(Vec3));
    normalize3(a);

    if (angle == 0) {
        load_id3(dest);
        return;
    }

    xv = ver * a[0];
    xs = a[0] * s;
    yv = ver * a[1];
    ys = a[1] * s;
    zv = ver * a[2];
    zs = a[2] * s;

    dest[0][0] = a[0] * xv + c;
    dest[0][1] = a[0] * yv + zs;
    dest[0][2] = a[0] * zv - ys;

    dest[1][0] = a[1] * xv - zs;
    dest[1][1] = a[1] * yv + c;
    dest[1][2] = a[1] * zv + xs;

    dest[2][0] = a[2] * xv + ys;
    dest[2][1] = a[2] * yv - xs;
    dest[2][2] = a[2] * zv + c;
}

void load_rot4(RESTRICT_MAT4(dest), RESTRICT_VEC3(axis), float angle) {
    Vec3 a;
    float s = sin(angle), c = cos(angle);
    float xv, xs, yv, ys, zv, zs, ver = 1 - c;

    memcpy(a, axis, sizeof(Vec3));
    normalize3(a);

    if (angle == 0) {
        load_id4(dest);
        return;
    }

    xv = ver * a[0];
    xs = a[0] * s;
    yv = ver * a[1];
    ys = a[1] * s;
    zv = ver * a[2];
    zs = a[2] * s;

    dest[0][0] = a[0] * xv + c;
    dest[0][1] = a[0] * yv + zs;
    dest[0][2] = a[0] * zv - ys;
    dest[0][3] = 0.0;

    dest[1][0] = a[1] * xv - zs;
    dest[1][1] = a[1] * yv + c;
    dest[1][2] = a[1] * zv + xs;
    dest[1][3] = 0.0;

    dest[2][0] = a[2] * xv + ys;
    dest[2][1] = a[2] * yv - xs;
    dest[2][2] = a[2] * zv + c;
    dest[2][3] = 0.0;

    dest[3][0] = 0.0;
    dest[3][1] = 0.0;
    dest[3][2] = 0.0;
    dest[3][3] = 1.0;
}

void compute_rotation(Vec3 u, Vec3 v, Vec3 axis, float* angle) {
    float c, d = norm3(u) * norm3(v);
    if (d == 0.0) {
        *angle = 0.0;
        axis[0] = 0;
        axis[1] = 0;
        axis[2] = 0;
    } else {
        c = dot3(u, v) / d;
        cross3(axis, u, v);
        *angle = (c > 1.0) ? 0.0 : ((c < -1.0) ? M_PI : acos(c));
    }
}

