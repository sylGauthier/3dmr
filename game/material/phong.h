#include <game/material/alpha.h>
#include <game/material/param.h>
#include <game/math/linear_algebra.h>
#include <game/render/material.h>
#include <game/render/shader.h>

#ifndef PHONG_H
#define PHONG_H

#define GAME_UID_PHONG 2

struct PhongMaterialParams {
    struct MatParamVec3 ambient, diffuse, specular;
    struct MatParamFloat shininess;
    GLuint normalMap;
    struct AlphaParams alpha;
};

void phong_material_params_init(struct PhongMaterialParams* p);
struct PhongMaterialParams* phong_material_params_new(void);
struct Material* phong_material_new(struct PhongMaterialParams* params);
int material_is_phong(const struct Material* material);

#endif
