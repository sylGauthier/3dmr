#define GLSL

#include "ambient_light.h"
#include "directional_light.h"
#include "point_light.h"
#include "spot_light.h"

uniform sampler2D directionalLightDepthMap[MAX_DIRECTIONAL_LIGHTS];

layout(std140) uniform Lights {
    AmbientLight ambientLight;
    DirectionalLight directionalLights[MAX_DIRECTIONAL_LIGHTS];
    PointLight pointLights[MAX_POINT_LIGHTS];
    SpotLight spotLights[MAX_SPOT_LIGHTS];
    uint numDirectionalLights;
    uint numPointLights;
    uint numSpotLights;
};
