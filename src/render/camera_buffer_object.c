#include <stdlib.h>
#include <3dmr/render/camera_buffer_object.h>
#include <3dmr/render/shader.h>

/* offsets and size of uniform Camera (as defined in camera.glsl) following the std140 layout rules */
#define OFFSET_PROJECTION 0
#define SIZEOF_PROJECTION sizeof(Mat4)
#define OFFSET_VIEW (OFFSET_PROJECTION + SIZEOF_PROJECTION)
#define SIZEOF_VIEW sizeof(Mat4)
#define OFFSET_POSITION (OFFSET_VIEW + SIZEOF_VIEW)
#define SIZEOF_POSITION sizeof(Vec4)
#define SIZEOF_CAMERA_UBO (OFFSET_POSITION + SIZEOF_POSITION)

int camera_buffer_object_gen(struct UniformBuffer* dest) {
    if (!uniform_buffer_gen(SIZEOF_CAMERA_UBO, dest)) return 0;
    glBindBufferRange(GL_UNIFORM_BUFFER, CAMERA_UBO_BINDING, dest->ubo, 0, SIZEOF_CAMERA_UBO);
    return 1;
}

struct UniformBuffer* camera_buffer_object_new(void) {
    struct UniformBuffer* u;
    if ((u = malloc(sizeof(*u)))) {
        if (camera_buffer_object_gen(u)) {
            return u;
        }
        free(u);
    }
    return NULL;
}

void camera_buffer_object_default_init(struct UniformBuffer* dest, float ratio) {
    Mat4 tmp;
    Quaternion orientation = {1, 0, 0, 0};
    Vec3 position = {0, 0, 10};
    camera_view(position, orientation, tmp);
    camera_buffer_object_update_view_and_position(dest, MAT_CONST_CAST(tmp));
    camera_projection(ratio, 1.04, 0.1, 2000, tmp);
    camera_buffer_object_update_projection(dest, MAT_CONST_CAST(tmp));
}

void camera_buffer_object_update_projection(struct UniformBuffer* u, const Mat4 projection) {
    uniform_buffer_update(u, OFFSET_PROJECTION, sizeof(Mat4), projection);
}

void camera_buffer_object_update_view(struct UniformBuffer* u, const Mat4 view) {
    uniform_buffer_update(u, OFFSET_VIEW, sizeof(Mat4), view);
}

void camera_buffer_object_update_position(struct UniformBuffer* u, const Vec3 position) {
    uniform_buffer_update(u, OFFSET_POSITION, sizeof(Vec3), position);
}

void camera_buffer_object_update_view_and_position(struct UniformBuffer* u, const Mat4 view) {
    Vec3 position;
    camera_get_position(view, position);
    camera_buffer_object_update_view(u, view);
    camera_buffer_object_update_position(u, position);
}
