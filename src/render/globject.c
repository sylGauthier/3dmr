#include <game/render/globject.h>
#include <game/render/material.h>

void globject_render(const struct GLObject* glObject, Mat4 model, Mat3 inverseNormal) {
    struct Material* material = glObject->material;

    glUseProgram(material->program);
    glBindVertexArray(glObject->vertexArray->vao);
    glUniformMatrix4fv(glGetUniformLocation(material->program, "model"), 1, GL_FALSE, &model[0][0]);
    glUniformMatrix3fv(glGetUniformLocation(material->program, "inverseNormal"), 1, GL_FALSE, &inverseNormal[0][0]);
    glPolygonMode(GL_FRONT_AND_BACK, material->polygonMode);
    material->load(glObject->material);
    if (glObject->vertexArray->numIndices) {
        glDrawElements(GL_TRIANGLES, glObject->vertexArray->numIndices, GL_UNSIGNED_INT, 0);
    } else {
        glDrawArrays(GL_TRIANGLES, 0, glObject->vertexArray->numVertices);
    }
    glBindVertexArray(0);
    glUseProgram(0);
}
