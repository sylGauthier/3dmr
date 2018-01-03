#include "geometry.h"

#ifndef SOLID_COLOR_H
#define SOLID_COLOR_H

struct SolidColorGeometry {
    struct Geometry geometry;
    Vec3 color;
};

void solid_color_geometry(struct SolidColorGeometry* dest, const struct GLObject* glObject, float r, float g, float b);

void solid_color_shader_free(void);

#endif
