#ifndef LINEAR_ALGEBRA_H
#define LINEAR_ALGEBRA_H

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

extern const Vec3 VEC3_AXIS_X;
extern const Vec3 VEC3_AXIS_Y;
extern const Vec3 VEC3_AXIS_Z;

void normalize2(Vec2 v);
void mul2sv(Vec2 dest, float s, Vec2 v);
float dot2(Vec2 v1, Vec2 v2);

void add3v(RESTRICT_VEC3(dest), RESTRICT_VEC3(u), RESTRICT_VEC3(v));
void incr3v(RESTRICT_VEC3(dest), RESTRICT_VEC3(incr));
void sub3v(RESTRICT_VEC3(dest), RESTRICT_VEC3(u), RESTRICT_VEC3(v));
void decr3v(RESTRICT_VEC3(dest), RESTRICT_VEC3(decr));
void mul3sv(RESTRICT_VEC3(dest), float s, RESTRICT_VEC3(v));
void scale3v(Vec3 dest, float s);
void mul3mv(RESTRICT_VEC3(dest), RESTRICT_MAT3(m), RESTRICT_VEC3(v));
void mul3mm(RESTRICT_MAT3(dest), RESTRICT_MAT3(a), RESTRICT_MAT3(b));
void neg3v(Vec3 v);
void neg3m(Mat3 m);
void transpose3m(Mat3 m);
float det3(Mat3 m);
int invert3m(RESTRICT_MAT3(dest), RESTRICT_MAT3(src));
float norm3sq(Vec3 v);
float norm3(Vec3 v);
void normalize3(Vec3 v);
float dot3(Vec3 u, Vec3 v);
void cross3(Vec3 dest, Vec3 u, Vec3 v);
void row3m3(RESTRICT_VEC3(dest), RESTRICT_MAT3(m), unsigned int row);
void row3m4(RESTRICT_VEC3(dest), RESTRICT_MAT4(m), unsigned int row);
void load_id3(Mat3 dest);
void print3v(Vec3 v);
void print3m(Mat3 m);

void vec4to3(RESTRICT_VEC3(dest), RESTRICT_VEC3(src));
void mat4to3(RESTRICT_MAT3(dest), RESTRICT_MAT4(src));
void mat3to4(RESTRICT_MAT4(dest), RESTRICT_MAT3(src));

void add4v(RESTRICT_VEC4(dest), RESTRICT_VEC4(u), RESTRICT_VEC4(v));
void mul4sv(RESTRICT_VEC4(dest), float s, RESTRICT_VEC4(v));
void mul4sm(RESTRICT_MAT4(dest), float s, RESTRICT_MAT4(m));
void scale4v(Vec4 dest, float s);
void mul4mv(RESTRICT_VEC4(res), RESTRICT_MAT4(m), RESTRICT_VEC4(v));
void mul4mm(RESTRICT_MAT4(res), RESTRICT_MAT4(a), RESTRICT_MAT4(b));
void neg4v(Vec4 v);
void neg4m(Mat4 m);
void transpose4m(Mat4 m);
float norm4sq(Vec4 v);
float norm4(Vec4 v);
void normalize4(Vec4 v);
float dot4(Vec4 u, Vec4 v);
void row4(RESTRICT_VEC4(dest), RESTRICT_MAT4(m), unsigned int row);
void load_id4(Mat4 dest);
void print4v(Vec4 v);
void print4m(Mat4 m);

void load_rot3(RESTRICT_MAT3(dest), RESTRICT_VEC3(axis), float angle);
void load_rot4(RESTRICT_MAT4(dest), RESTRICT_VEC3(axis), float angle);
void compute_rotation(Vec3 u, Vec3 v, Vec3 axis, float* angle);

#endif
