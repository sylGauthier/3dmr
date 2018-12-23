#include <game/render/material.h>

#ifndef SOLID_H
#define SOLID_H

struct SolidColorMaterial {
    struct Material material;
    Vec3 color;
};

struct SolidTextureMaterial {
    struct Material material;
    GLuint texture;
};

struct SolidColorMaterial* solid_color_material_new(float r, float g, float b);
struct SolidTextureMaterial* solid_texture_material_new(GLuint texture);

#endif
