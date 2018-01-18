#include "geometry.h"

void geometry_render(const struct Geometry* geometry, const struct Camera* camera, const struct Lights* lights, Mat4 model, Mat3 inverseNormal) {
    glUseProgram(geometry->shader);
    glBindVertexArray(geometry->glObject.vao);
    glUniformMatrix4fv(glGetUniformLocation(geometry->shader, "projection"), 1, GL_FALSE, (float*)camera->projection);
    glUniformMatrix4fv(glGetUniformLocation(geometry->shader, "view"), 1, GL_FALSE, (float*)camera->view);
    glUniformMatrix4fv(glGetUniformLocation(geometry->shader, "model"), 1, GL_FALSE, (float*)model);
    glUniformMatrix3fv(glGetUniformLocation(geometry->shader, "inverseNormal"), 1, GL_FALSE, (float*)inverseNormal);
    glPolygonMode(GL_FRONT_AND_BACK, geometry->mode);
    if (geometry->prerender) {
        geometry->prerender(geometry, camera, lights);
    }
    if (geometry->glObject.numIndices) {
        glDrawElements(GL_TRIANGLES, geometry->glObject.numIndices, GL_UNSIGNED_INT, 0);
    } else {
        glDrawArrays(GL_TRIANGLES, 0, geometry->glObject.numVertices);
    }
    if (geometry->postrender) {
        geometry->postrender(geometry, camera, lights);
    }
    glBindVertexArray(0);
    glUseProgram(0);
}
