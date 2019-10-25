#include <game/material/param.h>
#include <game/render/material.h>

#ifndef SOLID_H
#define SOLID_H

#define GAME_UID_SOLID 1

struct SolidMaterial {
    struct Material material;
    struct MatParamVec3 color;
};

enum SolidMaterialFlags {
    SOLID_TEXTURED = 1 << 0,
    SOLID_OVERLAY = 1 << 1
};

struct SolidMaterial* solid_material_new(enum SolidMaterialFlags flags);
int material_is_solid(const struct Material* material);

#endif
