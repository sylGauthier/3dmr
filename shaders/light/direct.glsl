#define GLSL

#include "ambient_light.h"
#include "directional_light.h"
#include "point_light.h"

layout(std140) uniform Lights {
    AmbientLight ambientLight;
    DirectionalLight directionalLights[MAX_DIRECTIONAL_LIGHTS];
    PointLight pointLights[MAX_POINT_LIGHTS];
    uint numDirectionalLights;
    uint numPointLights;
};
