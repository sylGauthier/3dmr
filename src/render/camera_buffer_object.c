#include <game/render/camera_buffer_object.h>

/* offsets and size of uniform Camera (as defined in camera.glsl) following the std140 layout rules */
#define OFFSET_PROJECTION 0
#define SIZEOF_PROJECTION sizeof(Mat4)
#define OFFSET_VIEW (OFFSET_PROJECTION + SIZEOF_PROJECTION)
#define SIZEOF_VIEW sizeof(Mat4)
#define OFFSET_POSITION (OFFSET_VIEW + SIZEOF_VIEW)
#define SIZEOF_POSITION sizeof(Vec4)
#define SIZEOF_CAMERA_UBO (OFFSET_POSITION + SIZEOF_POSITION)

GLuint camera_buffer_object(void) {
    GLuint ubo;

    glGenBuffers(1, &ubo);
    if (!ubo) return 0;
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferData(GL_UNIFORM_BUFFER, SIZEOF_CAMERA_UBO, NULL, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBufferRange(GL_UNIFORM_BUFFER, CAMERA_UBO_BINDING, ubo, 0, SIZEOF_CAMERA_UBO);
    return ubo;
}

void camera_buffer_object_update_projection(const Mat4 projection, GLuint ubo) {
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, OFFSET_PROJECTION, sizeof(Mat4), projection);
}

void camera_buffer_object_update_view(const Mat4 view, GLuint ubo) {
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, OFFSET_VIEW, sizeof(Mat4), view);
}

void camera_buffer_object_update_position(const Vec3 position, GLuint ubo) {
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, OFFSET_POSITION, sizeof(Vec3), position);
}

void camera_buffer_object_update(const struct Camera* camera, GLuint ubo) {
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, OFFSET_PROJECTION, sizeof(Mat4), camera->projection);
    glBufferSubData(GL_UNIFORM_BUFFER, OFFSET_VIEW, sizeof(Mat4), camera->view);
    glBufferSubData(GL_UNIFORM_BUFFER, OFFSET_POSITION, sizeof(Vec3), camera->position);
}
