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

struct PBRMaterial* pbr_material_new(enum MatParamMode modeAlbedo, enum MatParamMode modeMetalness, enum MatParamMode modeRoughness, int hasNormalMap);

#endif
