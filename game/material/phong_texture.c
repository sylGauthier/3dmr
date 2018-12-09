#include <stdlib.h>
#include <game/asset_manager.h>
#include "phong_texture.h"
#include "shaders.h"

static void phong_texture_prerender(const struct Material* material, const struct Camera* camera, const struct Lights* lights) {
    glBindTexture(GL_TEXTURE_2D, ((const struct PhongTextureMaterial*)material)->texture);
    light_load_uniforms(material->shader, lights);
    phong_material_load_uniform(material->shader, &((const struct PhongTextureMaterial*)material)->phong);
}

static void phong_texture_postrender(const struct Material* material, const struct Camera* camera, const struct Lights* lights) {
    glBindTexture(GL_TEXTURE_2D, 0);
}

struct PhongTextureMaterial* phong_texture_material_new(GLuint texture, const struct PhongMaterial* phong) {
    struct PhongTextureMaterial* phongTexture;

    if (!game_shaders[SHADER_PHONG_TEXTURE]) {
        if (!(game_shaders[SHADER_PHONG_TEXTURE] = asset_manager_load_shader("shaders/phong_texture.vert", "shaders/phong_texture.frag"))) {
            return NULL;
        }
    }
    if (!(phongTexture = malloc(sizeof(*phongTexture)))) {
        return NULL;
    }
    phongTexture->material.prerender = phong_texture_prerender;
    phongTexture->material.postrender = phong_texture_postrender;
    phongTexture->material.shader = game_shaders[SHADER_PHONG_TEXTURE];
    phongTexture->material.polygonMode = GL_FILL;
    phongTexture->phong = *phong;
    phongTexture->texture = texture;

    return phongTexture;
}
