#include <stdlib.h>
#include <game/asset_manager.h>
#include "solid.h"
#include "shaders.h"

static void solid_color_prerender(const struct Material* material, const struct Camera* camera, const struct Lights* lights) {
    glUniform3fv(glGetUniformLocation(material->shader, "solidColor"), 1, ((const struct SolidColorMaterial*)material)->color);
}

struct SolidColorMaterial* solid_color_material_new(float r, float g, float b) {
    struct SolidColorMaterial* solidColor;

    if (!game_shaders[SHADER_SOLID_COLOR]) {
        if (!(game_shaders[SHADER_SOLID_COLOR] = asset_manager_load_shader("shaders/solid_color.vert", "shaders/solid_color.frag"))) {
            return NULL;
        }
    }
    if (!(solidColor = malloc(sizeof(*solidColor)))) {
        return NULL;
    }
    solidColor->material.prerender = solid_color_prerender;
    solidColor->material.postrender = NULL;
    solidColor->material.shader = game_shaders[SHADER_SOLID_COLOR];
    solidColor->material.polygonMode = GL_FILL;
    solidColor->color[0] = r;
    solidColor->color[1] = g;
    solidColor->color[2] = b;

    return solidColor;
}

static void solid_texture_prerender(const struct Material* material, const struct Camera* camera, const struct Lights* lights) {
    glBindTexture(GL_TEXTURE_2D, ((const struct SolidTextureMaterial*)material)->texture);
}

static void solid_texture_postrender(const struct Material* material, const struct Camera* camera, const struct Lights* lights) {
    glBindTexture(GL_TEXTURE_2D, 0);
}

struct SolidTextureMaterial* solid_texture_material_new(GLuint texture) {
    struct SolidTextureMaterial* solidTexture;

    if (!game_shaders[SHADER_SOLID_TEXTURE]) {
        if (!(game_shaders[SHADER_SOLID_TEXTURE] = asset_manager_load_shader("shaders/solid_texture.vert", "shaders/solid_texture.frag"))) {
            return NULL;
        }
    }
    if (!(solidTexture = malloc(sizeof(*solidTexture)))) {
        return NULL;
    }
    solidTexture->material.prerender = solid_texture_prerender;
    solidTexture->material.postrender = solid_texture_postrender;
    solidTexture->material.shader = game_shaders[SHADER_SOLID_TEXTURE];
    solidTexture->material.polygonMode = GL_FILL;
    solidTexture->texture = texture;

    return solidTexture;
}
