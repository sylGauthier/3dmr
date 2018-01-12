#include "geometry.h"

void geometry_render(const struct Geometry* geometry, const struct Camera* camera, Mat4 model) {
    glUseProgram(geometry->shader);
    glBindVertexArray(geometry->glObject.vao);
    glUniformMatrix4fv(glGetUniformLocation(geometry->shader, "projection"), 1, GL_FALSE, (float*)camera->projection);
    glUniformMatrix4fv(glGetUniformLocation(geometry->shader, "view"), 1, GL_FALSE, (float*)camera->view);
    glUniformMatrix4fv(glGetUniformLocation(geometry->shader, "model"), 1, GL_FALSE, (float*)model);
    geometry->render(geometry);
    glBindVertexArray(0);
    glUseProgram(0);
}
