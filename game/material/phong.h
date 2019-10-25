#include <game/material/alpha.h>
#include <game/material/param.h>
#include <game/math/linear_algebra.h>
#include <game/render/material.h>
#include <game/render/shader.h>

#ifndef PHONG_H
#define PHONG_H

#define GAME_UID_PHONG 2

struct PhongMaterial {
    struct Material material;
    struct MatParamVec3 ambient, diffuse, specular;
    struct MatParamFloat shininess;
    GLuint normalMap;
    struct AlphaParams alpha;
};

enum PhongMaterialFlags {
    PHONG_AMBIENT_TEXTURED = 1 << 0,
    PHONG_DIFFUSE_TEXTURED = 1 << 1,
    PHONG_SPECULAR_TEXTURED = 1 << 2,
    PHONG_SHININESS_TEXTURED = 1 << 3,
    PHONG_NORMALMAP = 1 << 4
};

struct PhongMaterial* phong_material_new(enum PhongMaterialFlags flags);
int material_is_phong(const struct Material* material);

#endif
