#include <game/material/alpha.h>
#include <game/material/param.h>
#include <game/render/material.h>

#ifndef PBR_H
#define PBR_H

struct PBRMaterialParams {
    struct MatParamVec3 albedo;
    struct MatParamFloat metalness;
    struct MatParamFloat roughness;
    GLuint normalMap;
    struct AlphaParams alpha;
    struct IBL* ibl;
};

void pbr_load(GLuint program, void* params);

void pbr_material_params_init(struct PBRMaterialParams* p);
struct PBRMaterialParams* pbr_material_params_new(void);
GLuint pbr_shader_new(const struct PBRMaterialParams* params);
int material_is_pbr(const struct Material* material);

#endif
