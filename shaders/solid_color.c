#include "solid_color.h"

void draw_solid_color(const struct GLObject* glo, GLuint shader, const struct Camera* camera, Mat4 model, Vec3 color) {
    glUseProgram(shader);
    glBindVertexArray(glo->vao);
    glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, (float*)camera->projection);
    glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE, (float*)camera->view);
    glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, (float*)model);
    glUniform3fv(glGetUniformLocation(shader, "solidColor"), 1, (float*)color);
    glDrawArrays(GL_TRIANGLES, 0, glo->numVertices);
    glUseProgram(0);
}
