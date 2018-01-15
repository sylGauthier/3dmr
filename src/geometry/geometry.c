#include "geometry.h"

void geometry_render(const struct Geometry* geometry, const struct Camera* camera, Mat4 model) {
    glUseProgram(geometry->shader);
    glBindVertexArray(geometry->glObject.vao);
    glUniformMatrix4fv(glGetUniformLocation(geometry->shader, "projection"), 1, GL_FALSE, (float*)camera->projection);
    glUniformMatrix4fv(glGetUniformLocation(geometry->shader, "view"), 1, GL_FALSE, (float*)camera->view);
    glUniformMatrix4fv(glGetUniformLocation(geometry->shader, "model"), 1, GL_FALSE, (float*)model);
    glPolygonMode(GL_FRONT_AND_BACK, geometry->mode);
    if (geometry->prerender) {
        geometry->prerender(geometry, camera);
    }
    if (geometry->glObject.numIndices) {
        glDrawElements(GL_TRIANGLES, geometry->glObject.numIndices, GL_UNSIGNED_INT, 0);
    } else {
        glDrawArrays(GL_TRIANGLES, 0, geometry->glObject.numVertices);
    }
    if (geometry->postrender) {
        geometry->postrender(geometry, camera);
    }
    glBindVertexArray(0);
    glUseProgram(0);
}
