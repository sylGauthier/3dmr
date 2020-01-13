#include <stdlib.h>
#include <game/render/material.h>
#include <game/render/shader.h>

struct Material* material_new(void (*vertex_load)(const struct VertexArray* va),
                              void (*load)(GLuint program, void* params),
                              const void* params,
                              GLuint program,
                              GLuint polygonMode) {
    struct Material* m;
    if ((m = malloc(sizeof(*m)))) {
        m->vertex_load = vertex_load;
        m->load = load;
        m->params = (void*)params;
        m->program = program;
        m->polygonMode = polygonMode;
    }
    return m;
}

struct Material* material_new_from_shaders(const GLuint* shaders,
                                           unsigned int numShaders,
                                           void (*vertex_load)(const struct VertexArray* va),
                                           void (*load)(GLuint program, void* params),
                                           const void* params,
                                           GLuint polygonMode) {
    GLuint program;
    if (!(program = shader_link(shaders, numShaders))) return 0;
    return material_new(vertex_load, load, params, program, polygonMode);
}

void material_use(const struct Material* material, const struct VertexArray* va) {
    glUseProgram(material->program);
    glPolygonMode(GL_FRONT_AND_BACK, material->polygonMode);
    material->vertex_load(va);
    material->load(material->program, material->params);
}

void material_set_matrices(const struct Material* material, Mat4 model, Mat3 inverseNormal) {
    glUniformMatrix4fv(glGetUniformLocation(material->program, "model"), 1, GL_FALSE, &model[0][0]);
    glUniformMatrix3fv(glGetUniformLocation(material->program, "inverseNormal"), 1, GL_FALSE, &inverseNormal[0][0]);
}
