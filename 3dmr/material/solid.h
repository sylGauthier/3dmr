#include <3dmr/material/alpha.h>
#include <3dmr/material/param.h>
#include <3dmr/mesh/mesh.h>
#include <3dmr/render/material.h>

#ifndef TDMR_MATERIAL_SOLID_H
#define TDMR_MATERIAL_SOLID_H

struct SolidMaterialParams {
    struct MatParamVec3 color;
    struct AlphaParams alpha;
    int floatTexture;
};

void solid_load(GLuint program, void* params);

void solid_material_params_init(struct SolidMaterialParams* p);
struct SolidMaterialParams* solid_material_params_new(void);
GLuint solid_shader_new(const struct SolidMaterialParams* params);
struct Material* solid_material_new(enum MeshFlags mflags, const struct SolidMaterialParams* params);
struct Material* solid_overlay_material_new(enum MeshFlags mflags, const struct SolidMaterialParams* params);
int material_is_solid(const struct Material* material);

#endif
