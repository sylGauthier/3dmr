#ifndef SPOT_LIGHT_H
#define SPOT_LIGHT_H

#define MAX_SPOT_LIGHTS 10

struct SpotLight {
    vec3 position;
    vec3 direction;
    vec3 color;
    float intensity;
    float outerAngle;
    float innerAngle;
};

#ifdef GLSL
float spot_light_attenuation(SpotLight light, vec3 position) {
    float distance = length(light.position - position);
    float cosMin, cosMax, cosCur;
    float factor = 1.;
    vec3 lightToPos = normalize(position - light.position);
    cosMax = cos(light.innerAngle);
    cosMin = cos(light.outerAngle);
    cosCur = dot(normalize(light.direction), lightToPos);
    if (cosCur <= cosMin) return 0.0;
    if (cosCur > cosMin && cosCur < cosMax) factor = (cosCur - cosMin) / (cosMax - cosMin);
    return factor / (1.0 + (2.0 / light.intensity) * distance + (1.0 / (light.intensity * light.intensity)) * distance * distance);
}
#endif

#endif
