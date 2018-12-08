#include <game/render/material.h>
#include <game/text.h>

#ifndef SOLID_TEXT_H
#define SOLID_TEXT_H

struct SolidTextMaterial {
    struct Material material;
    Vec3 color;
    const struct BitmapFont* font;
};

struct SolidTextMaterial* solid_text_material_new(float r, float g, float b, const struct BitmapFont* font);

#endif
