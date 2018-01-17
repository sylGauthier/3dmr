#include "geometry.h"
#include "../light/phong.h"

#ifndef PHONG_COLOR_H
#define PHONG_COLOR_H

struct PhongColorMaterial {
    Vec3 color;
    struct PhongMaterial phong;
};

struct Geometry* phong_color_geometry(const struct GLObject* glObject, float r, float g, float b, const struct PhongMaterial* phong);
struct Geometry* phong_color_geometry_shared(const struct GLObject* glObject, struct PhongColorMaterial* material);

void phong_color_shader_free(void);

#endif
