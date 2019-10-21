#include <game/material/param.h>
#include <game/render/material.h>

#ifndef PBR_H
#define PBR_H

struct PBRMaterial {
    struct Material material;
    struct MatParamVec3 albedo;
    struct MatParamFloat metalness;
    struct MatParamFloat roughness;
    GLuint normalMap;
    struct IBL* ibl;
};

enum PBRMaterialFlags {
    PBR_ALBEDO_TEXTURED = 1 << 0,
    PBR_METALNESS_TEXTURED = 1 << 1,
    PBR_ROUGHNESS_TEXTURED = 1 << 2,
    PBR_NORMALMAP = 1 << 3
};

struct PBRMaterial* pbr_material_new(enum PBRMaterialFlags flags);
int material_is_pbr(const struct Material* material);

#endif
