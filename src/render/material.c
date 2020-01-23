#include <stdlib.h>
#include <game/render/material.h>
#include <game/render/shader.h>

struct Material* material_new(void (*vload)(GLuint, void*), void (*fload)(GLuint, void*), GLuint program, GLuint polygonMode) {
    struct Material* m;
    if ((m = malloc(sizeof(*m)))) {
        m->vertex_load = vload;
        m->frag_load = fload;
        m->program = program;
        m->polygonMode = polygonMode;
    }
    return m;
}

struct Material* material_new_from_shaders(const GLuint* shaders, unsigned int numShaders, void (*vload)(GLuint, void*), void (*fload)(GLuint, void*), GLuint polygonMode) {
    GLuint program;
    if (!(program = shader_link(shaders, numShaders))) return 0;
    return material_new(vload, fload, program, polygonMode);
}

void material_use(const struct Material* material, const void* vertParams, const void* fragParams) {
    glUseProgram(material->program);
    glPolygonMode(GL_FRONT_AND_BACK, material->polygonMode);
    if (material->vertex_load) {
        material->vertex_load(material->program, (void*)vertParams);
    }
    if (material->frag_load) {
        material->frag_load(material->program, (void*)fragParams);
    }
}

void material_set_matrices(const struct Material* material, Mat4 model, Mat3 inverseNormal) {
    glUniformMatrix4fv(glGetUniformLocation(material->program, "model"), 1, GL_FALSE, &model[0][0]);
    glUniformMatrix3fv(glGetUniformLocation(material->program, "inverseNormal"), 1, GL_FALSE, &inverseNormal[0][0]);
}
