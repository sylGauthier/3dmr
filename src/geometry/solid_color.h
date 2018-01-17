#include "geometry.h"

#ifndef SOLID_COLOR_H
#define SOLID_COLOR_H

struct SolidColorMaterial {
    Vec3 color;
};

struct Geometry* solid_color_geometry(const struct GLObject* glObject, float r, float g, float b);
struct Geometry* solid_color_geometry_shared(const struct GLObject* glObject, struct SolidColorMaterial* material);

void solid_color_shader_free(void);

#endif
