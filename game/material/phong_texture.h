#include <game/render/material.h>
#include <game/material/phong.h>

#ifndef PHONG_TEXTURE_H
#define PHONG_TEXTURE_H

struct PhongTextureMaterial {
    struct Material material;
    struct PhongMaterial phong;
    GLuint texture;
};

struct PhongTextureMaterial* phong_texture_material_new(GLuint texture, const struct PhongMaterial* phong);

#endif
