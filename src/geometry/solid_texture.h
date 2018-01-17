#include "geometry.h"

#ifndef SOLID_TEXTURE_H
#define SOLID_TEXTURE_H

struct SolidTextureMaterial {
    GLuint texture;
};

struct Geometry* solid_texture_geometry(const struct GLObject* glObject, GLuint texture);
struct Geometry* solid_texture_geometry_shared(const struct GLObject* glObject, struct SolidTextureMaterial* material);

void solid_texture_shader_free(void);

#endif
