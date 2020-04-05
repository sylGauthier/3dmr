#include <GL/glew.h>
#include <game/math/linear_algebra.h>
#include <game/math/glsl.h>
#include <game/light/ambient_light.h>
#include <game/light/directional_light.h>
#include <game/light/point_light.h>
#include <game/light/spot_light.h>
#include <game/light/ibl.h>

#ifndef LIGHT_H
#define LIGHT_H

struct Lights {
    struct DirectionalLight directional[MAX_DIRECTIONAL_LIGHTS];
    struct PointLight point[MAX_POINT_LIGHTS];
    struct SpotLight spot[MAX_SPOT_LIGHTS];
    struct AmbientLight ambientLight;
    unsigned int numDirectionalLights, numPointLights, numSpotLights;
};

void light_init(struct Lights* lights);

#endif
