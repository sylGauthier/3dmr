#include <game/math/linear_algebra.h>
#include <game/math/quaternion.h>

#ifndef CAMERA_H
#define CAMERA_H

struct Camera {
    Vec3 position;
    Quaternion orientation;
    float ratio, fov, zNear, zFar;

    Mat4 projection, view;
};

void camera_load_default(struct Camera* camera, const Vec3 pos, float ratio);

void camera_get_right(const struct Camera* camera, Vec3 right);
void camera_get_up(const struct Camera* camera, Vec3 up);
void camera_get_backward(const struct Camera* camera, Vec3 backward);

void camera_move(struct Camera* camera, const Vec3 translation);
void camera_rotate(struct Camera* camera, const Vec3 axis, float angle);

void camera_update_view(struct Camera* camera);
void camera_update_projection(struct Camera* camera);

#endif
