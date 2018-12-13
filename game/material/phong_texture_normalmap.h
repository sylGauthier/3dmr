#include <game/render/material.h>
#include <game/material/phong.h>

#ifndef PHONG_TEXTURE_NORMALMAP_H
#define PHONG_TEXTURE_NORMALMAP_H

struct PhongTextureNormalmapMaterial {
    struct Material material;
    struct PhongMaterial phong;
    GLuint texture;
    GLuint normalMap;
};

struct PhongTextureNormalmapMaterial* phong_texture_normalmap_material_new(GLuint texture, GLuint normalMap, const struct PhongMaterial* phong);

#endif
