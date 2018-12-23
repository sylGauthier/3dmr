#include <game/math/linear_algebra.h>
#include <game/render/material.h>
#include <game/render/shader.h>
#include <shaders/material/phong.h>

#ifndef PHONG_H_
#define PHONG_H_

struct PhongColorMaterial {
    struct Material material;
    struct PhongMaterial phong;
    Vec3 color;
};

struct PhongTextureMaterial {
    struct Material material;
    struct PhongMaterial phong;
    GLuint texture;
};

struct PhongTextureNormalmapMaterial {
    struct Material material;
    struct PhongMaterial phong;
    GLuint texture;
    GLuint normalMap;
};

struct PhongColorMaterial* phong_color_material_new(float r, float g, float b, const struct PhongMaterial* phong);
struct PhongTextureMaterial* phong_texture_material_new(GLuint texture, const struct PhongMaterial* phong);
struct PhongTextureNormalmapMaterial* phong_texture_normalmap_material_new(GLuint texture, GLuint normalMap, const struct PhongMaterial* phong);

void phong_material_load_uniform(GLuint shader, const struct PhongMaterial* material);

#endif
