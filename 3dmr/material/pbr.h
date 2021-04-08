#include <3dmr/material/alpha.h>
#include <3dmr/material/param.h>
#include <3dmr/mesh/mesh.h>
#include <3dmr/render/material.h>

#ifndef TDMR_MATERIAL_PBR_H
#define TDMR_MATERIAL_PBR_H

struct PBRMaterialParams {
    struct MatParamVec3 albedo;
    struct MatParamFloat metalness;
    struct MatParamFloat roughness;
    GLuint normalMap;
    GLuint occlusionMap;
    struct AlphaParams alpha;
    struct IBL* ibl;
};

void pbr_load(GLuint program, void* params);

void pbr_material_params_init(struct PBRMaterialParams* p);
struct PBRMaterialParams* pbr_material_params_new(void);
GLuint pbr_shader_new(const struct PBRMaterialParams* params);
struct Material* pbr_material_new(enum MeshFlags mflags, const struct PBRMaterialParams* params);
int material_is_pbr(const struct Material* material);

#endif
