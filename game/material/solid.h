#include <game/material/alpha.h>
#include <game/material/param.h>
#include <game/mesh/mesh.h>
#include <game/render/material.h>

#ifndef GAME_MATERIAL_SOLID_H
#define GAME_MATERIAL_SOLID_H

struct SolidMaterialParams {
    struct MatParamVec3 color;
    struct AlphaParams alpha;
};

void solid_load(GLuint program, void* params);

void solid_material_params_init(struct SolidMaterialParams* p);
struct SolidMaterialParams* solid_material_params_new(void);
GLuint solid_shader_new(const struct SolidMaterialParams* params);
struct Material* solid_material_new(enum MeshFlags mflags, const struct SolidMaterialParams* params);
struct Material* solid_overlay_material_new(enum MeshFlags mflags, const struct SolidMaterialParams* params);
int material_is_solid(const struct Material* material);

#endif
