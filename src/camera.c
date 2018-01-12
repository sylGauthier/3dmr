#include <string.h>
#include <math.h>
#include "linear_algebra.h"
#include "quaternion.h"
#include "camera.h"

static void camera_update_position(struct Camera* camera) {
    camera->view[3][0] = -(camera->view[0][0] * camera->position[0] + camera->view[1][0] * camera->position[1] + camera->view[2][0] * camera->position[2]);
    camera->view[3][1] = -(camera->view[0][1] * camera->position[0] + camera->view[1][1] * camera->position[1] + camera->view[2][1] * camera->position[2]);
    camera->view[3][2] = -(camera->view[0][2] * camera->position[0] + camera->view[1][2] * camera->position[1] + camera->view[2][2] * camera->position[2]);
}

static void camera_update_orientation(struct Camera* camera) {
    Mat3 rot;
    quaternion_to_mat3(rot, camera->orientation);
    transpose3m(rot);
    mat3to4(camera->view, rot);
}

void camera_load_default(struct Camera* camera, Vec3 pos, float ratio) {
    memcpy(camera->position, pos, sizeof(Vec3));
    quaternion_load_id(camera->orientation);
    load_id4(camera->view);
    load_id4(camera->projection);

    camera->fov = 1.04;
    camera->ratio = ratio;
    camera->zNear = 0.1;
    camera->zFar = 2000;

    camera_update_view(camera);
    camera_update_projection(camera);
}

void camera_get_right(struct Camera* camera, Vec3 right) {
    row3m4(right, camera->view, 0);
}

void camera_get_up(struct Camera* camera, Vec3 up) {
    row3m4(up, camera->view, 1);
}

void camera_get_backward(struct Camera* camera, Vec3 backward) {
    row3m4(backward, camera->view, 2);
}

void camera_move(struct Camera* camera, Vec3 translation) {
    incr3v(camera->position, translation);
    camera_update_orientation(camera);
    camera_update_position(camera);
}

void camera_rotate(struct Camera* camera, Vec3 axis, float angle) {
    Quaternion q, old;
    quaternion_set_axis_angle(q, axis, angle);
    memcpy(old, camera->orientation, sizeof(Quaternion));
    quaternion_mul(camera->orientation, q, old);
    camera_update_orientation(camera);
    camera_update_position(camera);
}

void camera_update_view(struct Camera* camera) {
    camera_update_orientation(camera);
    camera_update_position(camera);
}

void camera_update_projection(struct Camera* camera) {
    float tanHalfFov = tan(camera->fov / 2.0);

    memset(camera->projection, 0, sizeof(Mat4));
    camera->projection[0][0] = 1.0 / (camera->ratio * tanHalfFov);
    camera->projection[1][1] = 1.0 / (tanHalfFov);
    camera->projection[2][2] = -(camera->zFar + camera->zNear) / (camera->zFar - camera->zNear);
    camera->projection[2][3] = -1.0;
    camera->projection[3][2] = -(2.0 * camera->zFar * camera->zNear) / (camera->zFar - camera->zNear);
}
