#include <game/material/alpha.h>
#include <game/material/param.h>
#include <game/render/material.h>

#ifndef SOLID_H
#define SOLID_H

struct SolidMaterialParams {
    struct MatParamVec3 color;
    struct AlphaParams alpha;
    enum SolidMaterialOptions {
        SOLID_OVERLAY = 1 << 0
    } options;
};

void solid_load(GLuint program, void* params);

void solid_material_params_init(struct SolidMaterialParams* p);
struct SolidMaterialParams* solid_material_params_new(void);
GLuint solid_shader_new(const struct SolidMaterialParams* params);
int material_is_solid(const struct Material* material);

#endif
