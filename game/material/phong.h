#include <game/math/linear_algebra.h>
#include <game/render/shader.h>
#include <shaders/material/phong.h>

#ifndef PHONG_H_
#define PHONG_H_

void phong_material_load_uniform(GLuint shader, const struct PhongMaterial* material);

#endif
