#define GLSL

#include "ambient_light.h"
#include "directional_light.h"
#include "point_light.h"
#include "spot_light.h"
#include "shadowmap.h"

uniform sampler2D   shadowMaps[MAX_SHADOWMAPS];
uniform mat4        shadowViews[MAX_SHADOWMAPS];

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
    int smID = directionalLights[lightID].shadow;

    if (smID < 0) return 0.;
    lightSpacePos =  shadowViews[smID] * vec4(fragPos, 1);
    lightSpacePos /= lightSpacePos.w;
    lightSpacePos = lightSpacePos * 0.5 + 0.5;

    if (lightSpacePos.z >= 1) return 0.;

    /* GLSL 1.40 does not allow to access sampler arrays with non constant index,
     * hence this abomination
     */
    switch (smID) {
        case 0:
            return texture(shadowMaps[0], lightSpacePos.xy).r < lightSpacePos.z - bias ? 1. : 0.;
        case 1:
            return texture(shadowMaps[1], lightSpacePos.xy).r < lightSpacePos.z - bias ? 1. : 0.;
        case 2:
            return texture(shadowMaps[2], lightSpacePos.xy).r < lightSpacePos.z - bias ? 1. : 0.;
        case 3:
            return texture(shadowMaps[3], lightSpacePos.xy).r < lightSpacePos.z - bias ? 1. : 0.;
        case 4:
            return texture(shadowMaps[4], lightSpacePos.xy).r < lightSpacePos.z - bias ? 1. : 0.;
        default:
            return 0.;
    }
}
