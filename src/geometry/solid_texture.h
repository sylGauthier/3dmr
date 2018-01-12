#include "geometry.h"

#ifndef SOLID_TEXTURE_H
#define SOLID_TEXTURE_H

struct SolidTextureGeometry {
    struct Geometry geometry;
    GLuint texture;
};

void solid_texture_geometry(struct SolidTextureGeometry* dest, const struct GLObject* glObject, GLuint texture);

void solid_texture_shader_free(void);

#endif
