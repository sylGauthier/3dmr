#ifdef GLSL
#define Vec3 vec3
#else
#include <game/math/linear_algebra.h>
#endif

#ifndef POINT_LIGHT_H
#define POINT_LIGHT_H

#define MAX_POINT_LIGHTS 10

struct PointLight {
    Vec3 position;
    float intensity;
    float decay;

    Vec3 ambient;
    Vec3 diffuse;
    Vec3 specular;
};

#endif
