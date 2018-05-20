#include "geometry.h"
#include "../text.h"

#ifndef SOLID_TEXT_H
#define SOLID_TEXT_H

struct SolidTextMaterial {
    Vec3 color;
    const struct BitmapFont* font;
};

struct Geometry* solid_text_geometry(const struct GLObject* glObject, float r, float g, float b, const struct BitmapFont* font);
struct Geometry* solid_text_geometry_shared(const struct GLObject* glObject, struct SolidTextMaterial* material);

#endif
