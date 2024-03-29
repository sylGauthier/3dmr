#include <3dmr/math/constants.h>

#ifndef TDMR_MATH_LINEAR_ALGEBRA_H
#define TDMR_MATH_LINEAR_ALGEBRA_H

/* If c99 is available, use the restrict keyword
 * to allow the compiler to do vectorial optimizations
 */
#if __STDC_VERSION__ >= 199901L
#define RESTRICT_VEC2(var) float var[restrict 2]
#define RESTRICT_VEC3(var) float var[restrict 3]
#define RESTRICT_VEC4(var) float var[restrict 4]
#define RESTRICT_MAT2(var) float var[restrict 2][2]
#define RESTRICT_MAT3(var) float var[restrict 3][3]
#define RESTRICT_MAT4(var) float var[restrict 4][4]
#else
#define RESTRICT_VEC2(var) Vec2 var
#define RESTRICT_VEC3(var) Vec3 var
#define RESTRICT_VEC4(var) Vec4 var
#define RESTRICT_MAT2(var) Mat2 var
#define RESTRICT_MAT3(var) Mat3 var
#define RESTRICT_MAT4(var) Mat4 var
#endif

typedef float Vec2[2];
typedef float Vec3[3];
typedef float Vec4[4];
typedef Vec2 Mat2[2];
typedef Vec3 Mat3[3];
typedef Vec4 Mat4[4];

#define MAT_CONST_CAST(m) ((const float(*)[])(m))

extern const Vec3 VEC3_AXIS_X;
extern const Vec3 VEC3_AXIS_Y;
extern const Vec3 VEC3_AXIS_Z;

/* Vec2 */
void zero2v(Vec2 v);
void print2v(const Vec2 v);
void add2v(RESTRICT_VEC2(dest), const RESTRICT_VEC2(u), const RESTRICT_VEC2(v));
void incr2v(RESTRICT_VEC2(dest), const RESTRICT_VEC2(incr));
void sub2v(RESTRICT_VEC2(dest), const RESTRICT_VEC2(u), const RESTRICT_VEC2(v));
void decr2v(RESTRICT_VEC2(dest), const RESTRICT_VEC2(decr));
void mul2sv(RESTRICT_VEC2(dest), float s, const RESTRICT_VEC2(v));
void scale2v(Vec2 dest, float s);
void neg2v(Vec2 v);
float norm2sq(const Vec2 v);
float norm2(const Vec2 v);
void normalize2(Vec2 v);
float dot2(const RESTRICT_VEC2(u), const RESTRICT_VEC2(v));

/* Vec3 */
void zero3v(Vec3 v);
void print3v(const Vec3 v);
void add3v(RESTRICT_VEC3(dest), const RESTRICT_VEC3(u), const RESTRICT_VEC3(v));
void incr3v(RESTRICT_VEC3(dest), const RESTRICT_VEC3(incr));
void sub3v(RESTRICT_VEC3(dest), const RESTRICT_VEC3(u), const RESTRICT_VEC3(v));
void decr3v(RESTRICT_VEC3(dest), const RESTRICT_VEC3(decr));
void mul3sv(RESTRICT_VEC3(dest), float s, const RESTRICT_VEC3(v));
void scale3v(Vec3 dest, float s);
void neg3v(Vec3 v);
float norm3sq(const Vec3 v);
float norm3(const Vec3 v);
void normalize3(Vec3 v);
float dot3(const RESTRICT_VEC3(u), const RESTRICT_VEC3(v));
void cross3(RESTRICT_VEC3(dest), const RESTRICT_VEC3(u), const RESTRICT_VEC3(v));
void outer3(RESTRICT_MAT3(dest), const RESTRICT_VEC3(u), const RESTRICT_VEC3(v));

/* Vec4 */
void zero4v(Vec4 v);
void print4v(const Vec4 v);
void add4v(RESTRICT_VEC4(dest), const RESTRICT_VEC4(u), const RESTRICT_VEC4(v));
void incr4v(RESTRICT_VEC4(dest), const RESTRICT_VEC4(incr));
void sub4v(RESTRICT_VEC4(dest), const RESTRICT_VEC4(u), const RESTRICT_VEC4(v));
void decr4v(RESTRICT_VEC4(dest), const RESTRICT_VEC4(decr));
void mul4sv(RESTRICT_VEC4(dest), float s, const RESTRICT_VEC4(v));
void scale4v(Vec4 dest, float s);
void neg4v(Vec4 v);
float norm4sq(const Vec4 v);
float norm4(const Vec4 v);
void normalize4(Vec4 v);
float dot4(const RESTRICT_VEC4(u), const RESTRICT_VEC4(v));

/* Mat2 */
void load_id2(Mat2 dest);
void print2m(const Mat2 m);
void mul2sm(RESTRICT_MAT2(dest), float s, const RESTRICT_MAT2(m));
void mul2mv(RESTRICT_VEC2(dest), const RESTRICT_MAT2(m), const RESTRICT_VEC2(v));
void mul2mm(RESTRICT_MAT2(dest), const RESTRICT_MAT2(a), const RESTRICT_MAT2(b));
void neg2m(Mat2 m);
void transpose2m(Mat2 m);
float det2(const Mat2 m);
int invert2m(RESTRICT_MAT2(dest), const RESTRICT_MAT2(src));

/* Mat3 */
void load_id3(Mat3 dest);
void print3m(const Mat3 m);
void mul3sm(RESTRICT_MAT3(dest), float s, const RESTRICT_MAT3(m));
void mul3mv(RESTRICT_VEC3(dest), const RESTRICT_MAT3(m), const RESTRICT_VEC3(v));
void mul3mm(RESTRICT_MAT3(dest), const RESTRICT_MAT3(a), const RESTRICT_MAT3(b));
void add3mm(RESTRICT_MAT3(dest), const RESTRICT_MAT3(a), const RESTRICT_MAT3(b));
void neg3m(Mat3 m);
void transpose3m(Mat3 m);
float det3(const Mat3 m);
int invert3m(RESTRICT_MAT3(dest), const RESTRICT_MAT3(src));


/* Mat4 */
void load_id4(Mat4 dest);
void print4m(const Mat4 m);
void mul4sm(RESTRICT_MAT4(dest), float s, const RESTRICT_MAT4(m));
void mul4mv(RESTRICT_VEC4(res), const RESTRICT_MAT4(m), const RESTRICT_VEC4(v));
void mul4mm(RESTRICT_MAT4(res), const RESTRICT_MAT4(a), const RESTRICT_MAT4(b));
void mul4m3v(RESTRICT_VEC3(res), const RESTRICT_MAT4(m), const RESTRICT_VEC3(v));
void neg4m(Mat4 m);
void transpose4m(Mat4 m);
float det4(const Mat4 m);
int invert4m(RESTRICT_MAT4(dest), const RESTRICT_MAT4(src));

/* Conversions */
void row3m3(RESTRICT_VEC3(dest), const RESTRICT_MAT3(m), unsigned int row);
void row3m4(RESTRICT_VEC3(dest), const RESTRICT_MAT4(m), unsigned int row);
void row4m4(RESTRICT_VEC4(dest), const RESTRICT_MAT4(m), unsigned int row);
void vec4to3(RESTRICT_VEC3(dest), const RESTRICT_VEC4(src));
void mat4to3(RESTRICT_MAT3(dest), const RESTRICT_MAT4(src));
void mat3to4(RESTRICT_MAT4(dest), const RESTRICT_MAT3(src));

/* Rotations */
void load_rot3(RESTRICT_MAT3(dest), const RESTRICT_VEC3(axis), float angle);
void load_rot4(RESTRICT_MAT4(dest), const RESTRICT_VEC3(axis), float angle);
void compute_rotation(const RESTRICT_VEC3(u), const RESTRICT_VEC3(v), RESTRICT_VEC3(axis), float* angle);

/* Setters */
#define set2v(v, x, y)          (v)[0] = (x); (v)[1] = (y);
#define set3v(v, x, y, z)       (v)[0] = (x); (v)[1] = (y); (v)[2] = (z);
#define set4v(v, x, y, z, t)    (v)[0] = (x); (v)[1] = (y); (v)[2] = (z); (v)[3] = (t);

#endif
