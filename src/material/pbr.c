#include <stdlib.h>
#include <game/asset_manager.h>
#include <game/material/pbr.h>
#include "shaders.h"

static void pbr_load(const struct Material* material, const struct Camera* camera, const struct Lights* lights) {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, ((const struct PBRMaterial*)material)->albedoTex);
    glUniform1i(glGetUniformLocation(material->shader, "tex"), 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, ((const struct PBRMaterial*)material)->normalMap);
    glUniform1i(glGetUniformLocation(material->shader, "normalMap"), 1);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, ((const struct PBRMaterial*)material)->metalnessTex);
    glUniform1i(glGetUniformLocation(material->shader, "metalnessTex"), 2);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, ((const struct PBRMaterial*)material)->roughnessTex);
    glUniform1i(glGetUniformLocation(material->shader, "roughnessTex"), 3);
    light_load_direct_uniforms(material->shader, lights);
    light_load_ibl_uniforms(material->shader, lights, GL_TEXTURE4, GL_TEXTURE5, GL_TEXTURE6);
}

struct PBRMaterial* pbr_material_new(GLuint albedoTex, GLuint normalMap, GLuint metalnessTex, GLuint roughnessTex) {
    struct PBRMaterial* pbrMat;

    if (!game_shaders[SHADER_PBR]) {
        if (!(game_shaders[SHADER_PBR] = asset_manager_load_shader("shaders/texture_normalmap.vert", "shaders/pbr.frag"))) {
            return NULL;
        }
    }
    if (!(pbrMat = malloc(sizeof(*pbrMat)))) {
        return NULL;
    }
    pbrMat->material.load = pbr_load;
    pbrMat->material.shader = game_shaders[SHADER_PBR];
    pbrMat->material.polygonMode = GL_FILL;
    pbrMat->albedoTex = albedoTex;
    pbrMat->normalMap = normalMap;
    pbrMat->metalnessTex = metalnessTex;
    pbrMat->roughnessTex = roughnessTex;

    return pbrMat;
}

static void pbr_uni_load(const struct Material* material, const struct Camera* camera, const struct Lights* lights) {
    glUniform3fv(glGetUniformLocation(material->shader, "color"), 1, ((const struct PBRUniMaterial*)material)->color);
    glUniform1fv(glGetUniformLocation(material->shader, "metalness"), 1, &((const struct PBRUniMaterial*)material)->metalness);
    glUniform1fv(glGetUniformLocation(material->shader, "roughness"), 1, &((const struct PBRUniMaterial*)material)->roughness);
    light_load_direct_uniforms(material->shader, lights);
    light_load_ibl_uniforms(material->shader, lights, GL_TEXTURE0, GL_TEXTURE1, GL_TEXTURE2);
}

struct PBRUniMaterial* pbr_uni_material_new(float r, float g, float b, float metalness, float roughness) {
    struct PBRUniMaterial* pbrMat;

    if (!game_shaders[SHADER_PBR_UNI]) {
        if (!(game_shaders[SHADER_PBR_UNI] = asset_manager_load_shader("shaders/color.vert", "shaders/pbr_uni.frag"))) {
            return NULL;
        }
    }
    if (!(pbrMat = malloc(sizeof(*pbrMat)))) {
        return NULL;
    }
    pbrMat->material.load = pbr_uni_load;
    pbrMat->material.shader = game_shaders[SHADER_PBR_UNI];
    pbrMat->material.polygonMode = GL_FILL;
    pbrMat->color[0] = r;
    pbrMat->color[1] = g;
    pbrMat->color[2] = b;
    pbrMat->metalness = metalness;
    pbrMat->roughness = roughness;

    return pbrMat;
}
