#include <stdlib.h>
#include <game/render/material.h>
#include <game/render/shader.h>

struct Material* material_new(void (*vload)(void*), void (*fload)(GLuint, void*), const void* vparams, const void* fparams, GLuint program, GLuint polygonMode) {
    struct Material* m;
    if ((m = malloc(sizeof(*m)))) {
        m->vertex_load = vload;
        m->frag_load = fload;
        m->vparams = (void*)vparams;
        m->fparams = (void*)fparams;
        m->program = program;
        m->polygonMode = polygonMode;
    }
    return m;
}

struct Material* material_new_from_shaders(const GLuint* shaders, unsigned int numShaders, void (*vload)(void*), void (*fload)(GLuint, void*), const void* vparams, const void* fparams, GLuint polygonMode) {
    GLuint program;
    if (!(program = shader_link(shaders, numShaders))) return 0;
    return material_new(vload, fload, vparams, fparams, program, polygonMode);
}

void material_use(const struct Material* material) {
    glUseProgram(material->program);
    glPolygonMode(GL_FRONT_AND_BACK, material->polygonMode);
    if (material->vertex_load) {
        material->vertex_load(material->vparams);
    }
    if (material->frag_load) {
        material->frag_load(material->program, material->fparams);
    }
}

void material_set_matrices(const struct Material* material, Mat4 model, Mat3 inverseNormal) {
    glUniformMatrix4fv(glGetUniformLocation(material->program, "model"), 1, GL_FALSE, &model[0][0]);
    glUniformMatrix3fv(glGetUniformLocation(material->program, "inverseNormal"), 1, GL_FALSE, &inverseNormal[0][0]);
}
