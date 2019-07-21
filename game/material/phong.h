#include <game/material/param.h>
#include <game/math/linear_algebra.h>
#include <game/render/material.h>
#include <game/render/shader.h>

#ifndef PHONG_H
#define PHONG_H

struct PhongMaterial {
    struct Material material;
    struct MatParamVec3 ambient, diffuse, specular;
    struct MatParamFloat shininess;
    GLuint normalMap;
};

struct PhongMaterial* phong_material_new(enum MatParamMode modeAmbient, enum MatParamMode modeDiffuse, enum MatParamMode modeSpecular, enum MatParamMode modeShininess, int hasNormalMap);
int material_is_phong(const struct Material* material);

#endif
