#include <game/render/material.h>

#ifndef SOLID_COLOR_H
#define SOLID_COLOR_H

struct SolidColorMaterial {
    struct Material material;
    Vec3 color;
};

struct SolidColorMaterial* solid_color_material_new(float r, float g, float b);

void solid_color_load_uniform(const struct Material* material, const float* color);

#endif
