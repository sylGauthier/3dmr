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

void material_bind_shadowmaps(const struct Material* material, struct Lights* lights) {
    unsigned int i;
    GLint smLoc, svLoc;

    if ((smLoc = glGetUniformLocation(material->program, "shadowMaps")) < 0 || (svLoc = glGetUniformLocation(material->program, "shadowViews")) < 0) {
        return;
    }
    for (i = 0; i < MAX_SHADOWMAPS; i++) {
        struct ShadowMap* map = &lights->shadowMaps[i];

        if (map->fbo && map->tex) {
            Mat4 lightTransform;

            mul4mm(lightTransform, MAT_CONST_CAST(map->projection), MAT_CONST_CAST(map->view));
            glUniform1i(smLoc + i, TEX_SLOT_DIR_SHADOWMAP + i);
            glUniformMatrix4fv(svLoc + i, 1, GL_FALSE, &lightTransform[0][0]);
        }
    }
}
