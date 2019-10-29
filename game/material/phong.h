#include <game/material/alpha.h>
#include <game/material/param.h>
#include <game/render/material.h>

#ifndef PHONG_H
#define PHONG_H

struct PhongMaterialParams {
    struct MatParamVec3 ambient, diffuse, specular;
    struct MatParamFloat shininess;
    GLuint normalMap;
    struct AlphaParams alpha;
};

void phong_load(GLuint program, void* params);

void phong_material_params_init(struct PhongMaterialParams* p);
struct PhongMaterialParams* phong_material_params_new(void);
GLuint phong_shader_new(const struct PhongMaterialParams* params);
int material_is_phong(const struct Material* material);

#endif
