#include <GL/glew.h>
#include "camera.h"

#ifndef CAMERA_BUFFER_OBJECT_H
#define CAMERA_BUFFER_OBJECT_H

#define CAMERA_UBO_BINDING 0

GLuint camera_buffer_object(void);

void camera_buffer_object_update_projection(const Mat4 projection, GLuint ubo);
void camera_buffer_object_update_view(const Mat4 view, GLuint ubo);
void camera_buffer_object_update_position(const Vec3 position, GLuint ubo);
void camera_buffer_object_update(const struct Camera* camera, GLuint ubo);

#endif
