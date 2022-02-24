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

float pcf_shadow(sampler2D map, vec2 pos, float depth, float bias) {
    float shadow = 0;
    float texelSize = 1. / 1024;
    int x, y;

    for (x = -1; x <= 1; x++) {
        for (y = -1; y <= 1; y++) {
            float pcfDepth = texture(map, pos + vec2(x, y) * texelSize).r;

            shadow += depth - bias > pcfDepth ? 1. : 0.;
        }
    }
    return shadow / 9.;
}

float compute_shadow(vec3 fragPos, vec3 fragNormal, uint lightID) {
    vec4 lightSpacePos;
    vec3 projCoords;
    float bias = max(0.0005 * (1 - dot(fragNormal, directionalLights[lightID].direction)), 0.0005);
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
            return pcf_shadow(shadowMaps[0], lightSpacePos.xy, lightSpacePos.z, bias);
        case 1:
            return pcf_shadow(shadowMaps[1], lightSpacePos.xy, lightSpacePos.z, bias);
        case 2:
            return pcf_shadow(shadowMaps[2], lightSpacePos.xy, lightSpacePos.z, bias);
        case 3:
            return pcf_shadow(shadowMaps[3], lightSpacePos.xy, lightSpacePos.z, bias);
        case 4:
            return pcf_shadow(shadowMaps[4], lightSpacePos.xy, lightSpacePos.z, bias);
        default:
            return 0.;
    }
}
