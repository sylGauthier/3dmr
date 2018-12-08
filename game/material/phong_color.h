#include <game/render/material.h>
#include <game/material/phong.h>

#ifndef PHONG_COLOR_H
#define PHONG_COLOR_H

struct PhongColorMaterial {
    struct Material material;
    struct PhongMaterial phong;
    Vec3 color;
};

struct PhongColorMaterial* phong_color_material_new(float r, float g, float b, const struct PhongMaterial* phong);

#endif
