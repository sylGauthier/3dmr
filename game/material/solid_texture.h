#include <game/render/material.h>

#ifndef SOLID_TEXTURE_H
#define SOLID_TEXTURE_H

struct SolidTextureMaterial {
    struct Material material;
    GLuint texture;
};

struct SolidTextureMaterial* solid_texture_material_new(GLuint texture);

#endif
