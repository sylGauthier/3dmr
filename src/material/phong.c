#include <stdlib.h>
#include <game/material/phong.h>
#include <game/render/camera_buffer_object.h>
#include <game/render/lights_buffer_object.h>
#include "shaders.h"

static void phong_color_load(const struct Material* material, const struct Lights* lights) {
    glUniform3fv(glGetUniformLocation(material->shader, "color"), 1, ((const struct PhongColorMaterial*)material)->color);
    phong_material_load_uniform(material->shader, &((const struct PhongColorMaterial*)material)->phong);
}

struct PhongColorMaterial* phong_color_material_new(float r, float g, float b, const struct PhongMaterial* phong) {
    static const char* defines[] = {"HAVE_NORMAL", NULL};
    struct PhongColorMaterial* phongColor;

    if (!game_shaders[SHADER_PHONG_COLOR]) {
        if (!(game_shaders[SHADER_PHONG_COLOR] = game_load_shader("standard.vert", "phong.frag", defines, sizeof(defines) / (2 * sizeof(*defines))))) {
            return NULL;
        }
        glUniformBlockBinding(game_shaders[SHADER_PHONG_COLOR], glGetUniformBlockIndex(game_shaders[SHADER_PHONG_COLOR], "Camera"), CAMERA_UBO_BINDING);
        glUniformBlockBinding(game_shaders[SHADER_PHONG_COLOR], glGetUniformBlockIndex(game_shaders[SHADER_PHONG_COLOR], "Lights"), LIGHTS_UBO_BINDING);
    }
    if (!(phongColor = malloc(sizeof(*phongColor)))) {
        return NULL;
    }
    phongColor->material.load = phong_color_load;
    phongColor->material.shader = game_shaders[SHADER_PHONG_COLOR];
    phongColor->material.polygonMode = GL_FILL;
    phongColor->phong = *phong;
    phongColor->color[0] = r;
    phongColor->color[1] = g;
    phongColor->color[2] = b;

    return phongColor;
}

static void phong_texture_load(const struct Material* material, const struct Lights* lights) {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, ((const struct PhongTextureMaterial*)material)->texture);
    glUniform1i(glGetUniformLocation(material->shader, "tex"), 0);
    phong_material_load_uniform(material->shader, &((const struct PhongTextureMaterial*)material)->phong);
}

struct PhongTextureMaterial* phong_texture_material_new(GLuint texture, const struct PhongMaterial* phong) {
    static const char* defines[] = {"HAVE_NORMAL", NULL, "HAVE_TEXCOORD", NULL};
    struct PhongTextureMaterial* phongTexture;

    if (!game_shaders[SHADER_PHONG_TEXTURE]) {
        if (!(game_shaders[SHADER_PHONG_TEXTURE] = game_load_shader("standard.vert", "phong.frag", defines, sizeof(defines) / (2 * sizeof(*defines))))) {
            return NULL;
        }
        glUniformBlockBinding(game_shaders[SHADER_PHONG_TEXTURE], glGetUniformBlockIndex(game_shaders[SHADER_PHONG_TEXTURE], "Camera"), CAMERA_UBO_BINDING);
        glUniformBlockBinding(game_shaders[SHADER_PHONG_TEXTURE], glGetUniformBlockIndex(game_shaders[SHADER_PHONG_TEXTURE], "Lights"), LIGHTS_UBO_BINDING);
    }
    if (!(phongTexture = malloc(sizeof(*phongTexture)))) {
        return NULL;
    }
    phongTexture->material.load = phong_texture_load;
    phongTexture->material.shader = game_shaders[SHADER_PHONG_TEXTURE];
    phongTexture->material.polygonMode = GL_FILL;
    phongTexture->phong = *phong;
    phongTexture->texture = texture;

    return phongTexture;
}

static void phong_texture_normalmap_load(const struct Material* material, const struct Lights* lights) {
    glUniform1i(glGetUniformLocation(material->shader, "normalMap"), 1);
    glUniform1i(glGetUniformLocation(material->shader, "tex"), 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, ((const struct PhongTextureNormalmapMaterial*)material)->normalMap);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, ((const struct PhongTextureNormalmapMaterial*)material)->texture);
    phong_material_load_uniform(material->shader, &((const struct PhongTextureNormalmapMaterial*)material)->phong);
}

struct PhongTextureNormalmapMaterial* phong_texture_normalmap_material_new(GLuint texture, GLuint normalMap, const struct PhongMaterial* phong) {
    static const char* defines[] = {"HAVE_NORMAL", NULL, "HAVE_TEXCOORD", NULL, "HAVE_TANGENT", NULL};
    struct PhongTextureNormalmapMaterial* phongTexture;

    if (!game_shaders[SHADER_PHONG_TEXTURE_NORMALMAP]) {
        if (!(game_shaders[SHADER_PHONG_TEXTURE_NORMALMAP] = game_load_shader("standard.vert", "phong.frag", defines, sizeof(defines) / (2 * sizeof(*defines))))) {
            return NULL;
        }
        glUniformBlockBinding(game_shaders[SHADER_PHONG_TEXTURE_NORMALMAP], glGetUniformBlockIndex(game_shaders[SHADER_PHONG_TEXTURE_NORMALMAP], "Camera"), CAMERA_UBO_BINDING);
        glUniformBlockBinding(game_shaders[SHADER_PHONG_TEXTURE_NORMALMAP], glGetUniformBlockIndex(game_shaders[SHADER_PHONG_TEXTURE_NORMALMAP], "Lights"), LIGHTS_UBO_BINDING);
    }
    if (!(phongTexture = malloc(sizeof(*phongTexture)))) {
        return NULL;
    }
    phongTexture->material.load = phong_texture_normalmap_load;
    phongTexture->material.shader = game_shaders[SHADER_PHONG_TEXTURE_NORMALMAP];
    phongTexture->material.polygonMode = GL_FILL;
    phongTexture->phong = *phong;
    phongTexture->texture = texture;
    phongTexture->normalMap = normalMap;

    return phongTexture;
}

void phong_material_load_uniform(GLuint shader, const struct PhongMaterial* material) {
    glUniform3fv(glGetUniformLocation(shader, "material.ambient"), 1, material->ambient);
    glUniform3fv(glGetUniformLocation(shader, "material.diffuse"), 1, material->diffuse);
    glUniform3fv(glGetUniformLocation(shader, "material.specular"), 1, material->specular);
    glUniform1fv(glGetUniformLocation(shader, "material.shininess"), 1, &material->shininess);
}
