#include <game/render/material.h>

#ifndef PBR_H
#define PBR_H

struct PBRMaterial {
    struct Material material;
    GLuint albedoTex;
    GLuint normalMap;
    GLuint metalnessTex;
    GLuint roughnessTex;
};

struct PBRUniMaterial {
    struct Material material;
    vec3 albedo;
    float metalness;
    float roughness;
};

struct PBRMaterial* pbr_material_new(GLuint albedoTex, GLuint normalMap, GLuint metalnessTex, GLuint roughnessTex);
struct PBRUniMaterial* pbr_uni_material_new(float r, float g, float b, float metalness, float roughness);

#endif
