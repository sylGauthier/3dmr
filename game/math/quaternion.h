#include "linear_algebra.h"

#ifndef QUATERNION_H
#define QUATERNION_H

typedef Vec4 Quaternion;

void quaternion_load_id(Quaternion q);
void quaternion_set_axis_angle(Quaternion q, const Vec3 axis, float angle);
void quaternion_get_axis(const Quaternion q, Vec3 axis);
float quaternion_get_angle(const Quaternion q);
void quaternion_set_angle(Quaternion q, float angle);

void quaternion_mul(Quaternion dest, const Quaternion a, const Quaternion b);
void quaternion_conj(Quaternion q);
void quaternion_inv(Quaternion q);
void quaternion_compose(Vec3 dest, const Quaternion q, const Vec3 v);
void quaternion_to_mat3(Mat3 dest, const Quaternion q);
void quaternion_to_mat4(Mat4 dest, const Quaternion q);
void quaternion_from_xyz(Quaternion dest, const Vec3 src);
void quaternion_from_mat3(Quaternion dest, const Mat3 src);
void quaternion_from_mat4(Quaternion dest, const Mat4 src);
void quaternion_decompose_swing_twist(const Quaternion src, const Vec3 direction, Quaternion swing, Quaternion twist);
void quaternion_print(const Quaternion q);

#endif
