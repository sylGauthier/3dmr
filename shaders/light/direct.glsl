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

float compute_shadow(vec3 fragPos, vec3 fragNormal, uint lightID) {
    vec4 lightSpacePos;
    vec3 projCoords;
    float bias = max(0.005 * (1 - dot(fragNormal, directionalLights[lightID].direction)), 0.005);

    if (directionalLights[lightID].shadow == 0) return 0;
    lightSpacePos = directionalLights[lightID].projection * directionalLights[lightID].view * vec4(fragPos, 1);
    lightSpacePos /= lightSpacePos.w;
    lightSpacePos = lightSpacePos * 0.5 + 0.5;

    if (lightSpacePos.z >= 1) return 0;
    return texture(directionalLightDepthMap[lightID], lightSpacePos.xy).r < lightSpacePos.z - bias ? 1. : 0.;
}
