#include <stdlib.h>
#include <game/asset_manager.h>
#include "pbr.h"
#include "shaders.h"

static void pbr_prerender(const struct Material* material, const struct Camera* camera, const struct Lights* lights) {
    glUniform1i(glGetUniformLocation(material->shader, "roughnessTex"), 3);
    glUniform1i(glGetUniformLocation(material->shader, "metalnessTex"), 2);
    glUniform1i(glGetUniformLocation(material->shader, "normalMap"), 1);
    glUniform1i(glGetUniformLocation(material->shader, "tex"), 0);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, ((const struct PBRMaterial*)material)->roughnessTex);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, ((const struct PBRMaterial*)material)->metalnessTex);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, ((const struct PBRMaterial*)material)->normalMap);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, ((const struct PBRMaterial*)material)->albedoTex);
    light_load_uniforms(material->shader, lights);
}

static void pbr_postrender(const struct Material* material, const struct Camera* camera, const struct Lights* lights) {
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
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
    pbrMat->material.prerender = pbr_prerender;
    pbrMat->material.postrender = pbr_postrender;
    pbrMat->material.shader = game_shaders[SHADER_PBR];
    pbrMat->material.polygonMode = GL_FILL;
    pbrMat->albedoTex = albedoTex;
    pbrMat->normalMap = normalMap;
    pbrMat->metalnessTex = metalnessTex;
    pbrMat->roughnessTex = roughnessTex;

    return pbrMat;
}

static void pbr_uni_prerender(const struct Material* material, const struct Camera* camera, const struct Lights* lights) {
    glUniform3fv(glGetUniformLocation(material->shader, "color"), 1, ((const struct PBRUniMaterial*)material)->color);
    glUniform1fv(glGetUniformLocation(material->shader, "metalness"), 1, &((const struct PBRUniMaterial*)material)->metalness);
    glUniform1fv(glGetUniformLocation(material->shader, "roughness"), 1, &((const struct PBRUniMaterial*)material)->roughness);
    light_load_uniforms(material->shader, lights);
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
    pbrMat->material.prerender = pbr_uni_prerender;
    pbrMat->material.postrender = NULL;
    pbrMat->material.shader = game_shaders[SHADER_PBR_UNI];
    pbrMat->material.polygonMode = GL_FILL;
    pbrMat->color[0] = r;
    pbrMat->color[1] = g;
    pbrMat->color[2] = b;
    pbrMat->metalness = metalness;
    pbrMat->roughness = roughness;

    return pbrMat;
}
