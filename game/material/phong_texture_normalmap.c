#include <stdlib.h>
#include <game/asset_manager.h>
#include "phong_texture_normalmap.h"
#include "shaders.h"

static void phong_texture_prerender(const struct Material* material, const struct Camera* camera, const struct Lights* lights) {
    glUniform1i(glGetUniformLocation(material->shader, "normalMap"), 1);
    glUniform1i(glGetUniformLocation(material->shader, "tex"), 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, ((const struct PhongTextureNormalmapMaterial*)material)->normalMap);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, ((const struct PhongTextureNormalmapMaterial*)material)->texture);
    light_load_uniforms(material->shader, lights);
    phong_material_load_uniform(material->shader, &((const struct PhongTextureNormalmapMaterial*)material)->phong);
}

static void phong_texture_postrender(const struct Material* material, const struct Camera* camera, const struct Lights* lights) {
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

struct PhongTextureNormalmapMaterial* phong_texture_normalmap_material_new(GLuint texture, GLuint normalMap, const struct PhongMaterial* phong) {
    struct PhongTextureNormalmapMaterial* phongTexture;

    if (!game_shaders[SHADER_PHONG_TEXTURE_NORMALMAP]) {
        if (!(game_shaders[SHADER_PHONG_TEXTURE_NORMALMAP] = asset_manager_load_shader("shaders/phong_texture_normalmap.vert", "shaders/phong_texture_normalmap.frag"))) {
            return NULL;
        }
    }
    if (!(phongTexture = malloc(sizeof(*phongTexture)))) {
        return NULL;
    }
    phongTexture->material.prerender = phong_texture_prerender;
    phongTexture->material.postrender = phong_texture_postrender;
    phongTexture->material.shader = game_shaders[SHADER_PHONG_TEXTURE_NORMALMAP];
    phongTexture->material.polygonMode = GL_FILL;
    phongTexture->phong = *phong;
    phongTexture->texture = texture;
    phongTexture->normalMap = normalMap;

    return phongTexture;
}
