#include <string.h>
#include <math.h>
#include <game/math/linear_algebra.h>
#include <game/math/quaternion.h>
#include <game/render/camera.h>

void camera_get_position(const Mat4 view, Vec3 position) {
    Mat3 tmp;
    Vec3 vtmp;
    mat4to3(tmp, view);
    transpose3m(tmp);
    mul3sv(vtmp, -1, view[3]);
    mul3mv(position, MAT_CONST_CAST(tmp), vtmp);
}

void camera_get_orientation(const Mat4 view, Quaternion orientation) {
    Mat3 rot;
    mat4to3(rot, view);
    transpose3m(rot);
    quaternion_from_mat3(orientation, MAT_CONST_CAST(rot));
}

void camera_get_right(const Mat4 view, Vec3 right) {
    row3m4(right, view, 0);
}

void camera_get_up(const Mat4 view, Vec3 up) {
    row3m4(up, view, 1);
}

void camera_get_backward(const Mat4 view, Vec3 backward) {
    row3m4(backward, view, 2);
}

void camera_view(const Vec3 position, const Quaternion orientation, Mat4 view) {
    Mat3 rot;
    quaternion_to_mat3(rot, orientation);
    transpose3m(rot);
    mat3to4(view, MAT_CONST_CAST(rot));
    mul3mv(view[3], MAT_CONST_CAST(rot), position);
    neg3v(view[3]);
}

void camera_set_ratio(float ratio, Mat4 projection) {
    projection[0][0] = projection[1][1] / ratio;
}

void camera_projection(float ratio, float fov, float zNear, float zFar, Mat4 projection) {
    float tanHalfFov = tan(fov / 2.0);

    projection[0][0] = 1.0 / (ratio * tanHalfFov);
    projection[0][1] = 0;
    projection[0][2] = 0;
    projection[0][3] = 0;
    projection[1][0] = 0;
    projection[1][1] = 1.0 / (tanHalfFov);
    projection[1][2] = 0;
    projection[1][3] = 0;
    projection[2][0] = 0;
    projection[2][1] = 0;
    projection[2][2] = -(zFar + zNear) / (zFar - zNear);
    projection[2][3] = -1.0;
    projection[3][0] = 0;
    projection[3][1] = 0;
    projection[3][2] = -(2.0 * zFar * zNear) / (zFar - zNear);
    projection[3][3] = 0;
}
