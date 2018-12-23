#include <game/render/material.h>
#include "globject.h"

void globject_render(const struct GLObject* glObject, const struct Camera* camera, const struct Lights* lights, Mat4 model, Mat3 inverseNormal) {
    struct Material* material = glObject->material;

    glUseProgram(material->shader);
    glBindVertexArray(glObject->vertexArray->vao);
    glUniformMatrix4fv(glGetUniformLocation(material->shader, "projection"), 1, GL_FALSE, (float*)camera->projection);
    glUniformMatrix4fv(glGetUniformLocation(material->shader, "view"), 1, GL_FALSE, (float*)camera->view);
    glUniformMatrix4fv(glGetUniformLocation(material->shader, "model"), 1, GL_FALSE, (float*)model);
    glUniformMatrix3fv(glGetUniformLocation(material->shader, "inverseNormal"), 1, GL_FALSE, (float*)inverseNormal);
    glUniform3fv(glGetUniformLocation(material->shader, "cameraPosition"), 1, camera->position);
    glPolygonMode(GL_FRONT_AND_BACK, material->polygonMode);
    if (material->prerender) {
        material->prerender(glObject->material, camera, lights);
    }
    if (glObject->vertexArray->numIndices) {
        glDrawElements(GL_TRIANGLES, glObject->vertexArray->numIndices, GL_UNSIGNED_INT, 0);
    } else {
        glDrawArrays(GL_TRIANGLES, 0, glObject->vertexArray->numVertices);
    }
    if (material->postrender) {
        material->postrender(glObject->material, camera, lights);
    }
    glBindVertexArray(0);
    glUseProgram(0);
}
