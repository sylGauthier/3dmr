#include <GL/glew.h>
#include <3dmr/math/linear_algebra.h>
#include <3dmr/light/ambient_light.h>
#include <3dmr/light/directional_light.h>
#include <3dmr/light/point_light.h>
#include <3dmr/light/spot_light.h>
#include <3dmr/light/ibl.h>

#ifndef TDMR_LIGHT_H
#define TDMR_LIGHT_H

struct Lights {
    struct DirectionalLight directional[MAX_DIRECTIONAL_LIGHTS];
    struct PointLight point[MAX_POINT_LIGHTS];
    struct SpotLight spot[MAX_SPOT_LIGHTS];
    struct AmbientLight ambientLight;
    unsigned int numDirectionalLights, numPointLights, numSpotLights;
};

void light_init(struct Lights* lights);

#endif
