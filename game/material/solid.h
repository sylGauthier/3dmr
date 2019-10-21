#include <game/material/param.h>
#include <game/render/material.h>

#ifndef SOLID_H
#define SOLID_H

struct SolidMaterial {
    struct Material material;
    struct MatParamVec3 color;
};

enum SolidMaterialFlags {
    SOLID_TEXTURED = 1 << 0
};

struct SolidMaterial* solid_material_new(enum SolidMaterialFlags flags);
int material_is_solid(const struct Material* material);

#endif
