#ifndef LIGHT_H
#define LIGHT_H

#include <GL/glew.h>
#include <game/math/linear_algebra.h>
#include <game/light/ambient_light.h>
#include <game/light/directional_light.h>
#include <game/light/point_light.h>

struct Lights {
    struct DirectionalLight directional[MAX_DIRECTIONAL_LIGHTS];
    struct PointLight point[MAX_POINT_LIGHTS];
    struct AmbientLight ambientLight;
    unsigned int numDirectionalLights, numPointLights;
};

void light_load_uniforms(GLuint shader, const struct Lights* lights);

#endif
