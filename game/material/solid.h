#include <game/material/alpha.h>
#include <game/material/param.h>
#include <game/render/material.h>

#ifndef SOLID_H
#define SOLID_H

#define GAME_UID_SOLID 1

struct SolidMaterialParams {
    struct MatParamVec3 color;
    struct AlphaParams alpha;
    enum SolidMaterialOptions {
        SOLID_OVERLAY = 1 << 0
    } options;
};

void solid_material_params_init(struct SolidMaterialParams* p);
struct SolidMaterialParams* solid_material_params_new(void);
struct Material* solid_material_new(struct SolidMaterialParams* params);
int material_is_solid(const struct Material* material);

#endif
