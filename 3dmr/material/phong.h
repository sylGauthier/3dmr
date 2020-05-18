#include <3dmr/material/alpha.h>
#include <3dmr/material/param.h>
#include <3dmr/mesh/mesh.h>
#include <3dmr/render/material.h>

#ifndef TDMR_MATERIAL_PHONG_H
#define TDMR_MATERIAL_PHONG_H

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
struct Material* phong_material_new(enum MeshFlags mflags, const struct PhongMaterialParams* params);
int material_is_phong(const struct Material* material);

#endif
