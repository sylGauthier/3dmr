#ifndef LIGHT_H
#define LIGHT_H

#ifdef GLSL
#define Vec3 vec3
#else
#include <GL/glew.h>
#include "../linear_algebra.h"
#endif

#define MAX_DIRECTIONAL_LIGHTS 10
#define MAX_LOCAL_LIGHTS 10

struct DirectionalLight {
    Vec3 direction;

    Vec3 ambient;
    Vec3 diffuse;
    Vec3 specular;
};

struct LocalLight {
    Vec3 position;
    float intensity;

    Vec3 ambient;
    Vec3 diffuse;
    Vec3 specular;
};

#ifndef GLSL
struct Lights {
    struct DirectionalLight directional[MAX_DIRECTIONAL_LIGHTS];
    struct LocalLight local[MAX_LOCAL_LIGHTS];
    unsigned int numDirectional, numLocal;
};

void light_load_uniforms(GLuint shader, const struct DirectionalLight* directionalLights, unsigned int nbDirectionalLights, const struct LocalLight* localLights, unsigned int nbLocalLights);
#endif

#endif
