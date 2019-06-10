#include <stdlib.h>
#include <game/material/solid.h>
#include "shaders.h"

static void solid_color_load(const struct Material* material, const struct Camera* camera, const struct Lights* lights) {
    glUniform3fv(glGetUniformLocation(material->shader, "solidColor"), 1, ((const struct SolidColorMaterial*)material)->color);
}

struct SolidColorMaterial* solid_color_material_new(float r, float g, float b) {
    struct SolidColorMaterial* solidColor;

    if (!game_shaders[SHADER_SOLID_COLOR]) {
        if (!(game_shaders[SHADER_SOLID_COLOR] = game_load_shader("standard.vert", "solid.frag", NULL, 0))) {
            return NULL;
        }
    }
    if (!(solidColor = malloc(sizeof(*solidColor)))) {
        return NULL;
    }
    solidColor->material.load = solid_color_load;
    solidColor->material.shader = game_shaders[SHADER_SOLID_COLOR];
    solidColor->material.polygonMode = GL_FILL;
    solidColor->color[0] = r;
    solidColor->color[1] = g;
    solidColor->color[2] = b;

    return solidColor;
}

static void solid_texture_load(const struct Material* material, const struct Camera* camera, const struct Lights* lights) {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, ((const struct SolidTextureMaterial*)material)->texture);
    glUniform1i(glGetUniformLocation(material->shader, "tex"), 0);
}

struct SolidTextureMaterial* solid_texture_material_new(GLuint texture) {
    static const char* defines[] = {"HAVE_TEXCOORD", NULL};
    struct SolidTextureMaterial* solidTexture;

    if (!game_shaders[SHADER_SOLID_TEXTURE]) {
        if (!(game_shaders[SHADER_SOLID_TEXTURE] = game_load_shader("standard.vert", "solid.frag", defines, sizeof(defines) / (2 * sizeof(*defines))))) {
            return NULL;
        }
    }
    if (!(solidTexture = malloc(sizeof(*solidTexture)))) {
        return NULL;
    }
    solidTexture->material.load = solid_texture_load;
    solidTexture->material.shader = game_shaders[SHADER_SOLID_TEXTURE];
    solidTexture->material.polygonMode = GL_FILL;
    solidTexture->texture = texture;

    return solidTexture;
}
