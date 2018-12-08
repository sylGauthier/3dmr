#ifdef GLSL
#define Vec3 vec3
#else
#include <game/math/linear_algebra.h>
#include <game/render/shader.h>
#endif

#ifndef PHONG_H
#define PHONG_H

struct PhongMaterial {
    Vec3 ambient;
    Vec3 diffuse;
    Vec3 specular;
    float shininess;
};

#ifndef GLSL
void phong_material_load_uniform(GLuint shader, const struct PhongMaterial* material);
#endif

#endif
