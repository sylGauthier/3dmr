#include <game/math/linear_algebra.h>
#include <game/math/quaternion.h>

#ifndef CAMERA_H
#define CAMERA_H

void camera_get_position(const Mat4 view, Vec3 position);
void camera_get_orientation(const Mat4 view, Quaternion orientation);
void camera_get_right(const Mat4 view, Vec3 right);
void camera_get_up(const Mat4 view, Vec3 up);
void camera_get_backward(const Mat4 view, Vec3 backward);

void camera_view(const Vec3 position, const Quaternion orientation, Mat4 view);
void camera_projection(float ratio, float fov, float zNear, float zFar, Mat4 projection);

#endif
