#include <stdlib.h>
#include <3dmr/render/material.h>
#include <3dmr/render/shader.h>

struct Material* material_new(void (*load)(GLuint, void*), void* params, GLuint program, GLuint polygonMode) {
    struct Material* m;
    if ((m = malloc(sizeof(*m)))) {
        m->load = load;
        m->params = params;
        m->program = program;
        m->polygonMode = polygonMode;
    }
    return m;
}

struct Material* material_new_from_shaders(const GLuint* shaders, unsigned int numShaders, void (*load)(GLuint, void*), void* params, GLuint polygonMode) {
    GLuint program;
    if (!(program = shader_link(shaders, numShaders))) return 0;
    return material_new(load, params, program, polygonMode);
}

void material_use(const struct Material* material) {
    glUseProgram(material->program);
    glPolygonMode(GL_FRONT_AND_BACK, material->polygonMode);
    if (material->load) {
        material->load(material->program, material->params);
    }
}

void material_set_matrices(const struct Material* material, Mat4 model, Mat3 inverseNormal) {
    glUniformMatrix4fv(glGetUniformLocation(material->program, "model"), 1, GL_FALSE, &model[0][0]);
    glUniformMatrix3fv(glGetUniformLocation(material->program, "inverseNormal"), 1, GL_FALSE, &inverseNormal[0][0]);
}
