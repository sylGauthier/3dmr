#include "geometry.h"
#include "linear_algebra.h"

#ifndef SHADERS_SOLID_COLOR_H
#define SHADERS_SOLID_COLOR_H

struct SolidColorGeometry {
    struct Geometry geometry;
    Vec3 color;
};

void draw_solid_color(const struct Geometry* geometry);

#endif
