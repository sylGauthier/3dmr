#include "geometry.h"
#include "linear_algebra.h"

#ifndef SHADERS_SOLID_TEXTURE_H
#define SHADERS_SOLID_TEXTURE_H

struct SolidTextureGeometry {
    struct Geometry geometry;
    GLuint texture;
};

void draw_solid_texture(const struct Geometry* geometry);

#endif
