#include "geometry.h"
#include "../light/phong.h"

#ifndef PHONG_TEXTURE_H
#define PHONG_TEXTURE_H

struct PhongTextureMaterial {
    GLuint texture;
    struct PhongMaterial phong;
};

struct Geometry* phong_texture_geometry(const struct GLObject* glObject, GLuint texture, const struct PhongMaterial* phong);
struct Geometry* phong_texture_geometry_shared(const struct GLObject* glObject, struct PhongTextureMaterial* material);

void phong_texture_shader_free(void);

#endif
