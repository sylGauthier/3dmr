#ifdef GLSL
#define Vec3 vec3
#else
#include <game/math/linear_algebra.h>
#endif

#ifndef DIRECTIONAL_LIGHT_H
#define DIRECTIONAL_LIGHT_H

#define MAX_DIRECTIONAL_LIGHTS 10

struct DirectionalLight {
    Vec3 direction;

    Vec3 ambient;
    Vec3 diffuse;
    Vec3 specular;
};

#endif
