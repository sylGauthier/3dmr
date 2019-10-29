#include <game/render/uniform_buffer.h>
#include "camera.h"

#ifndef CAMERA_BUFFER_OBJECT_H
#define CAMERA_BUFFER_OBJECT_H

int camera_buffer_object_gen(struct UniformBuffer* dest);
struct UniformBuffer* camera_buffer_object_new(void);

void camera_buffer_object_default_init(struct UniformBuffer* dest, float ratio);

void camera_buffer_object_update_projection(struct UniformBuffer* u, const Mat4 projection);
void camera_buffer_object_update_view(struct UniformBuffer* u, const Mat4 view);
void camera_buffer_object_update_position(struct UniformBuffer* u, const Vec3 position);
void camera_buffer_object_update_view_and_position(struct UniformBuffer* u, const Mat4 view);

#endif
