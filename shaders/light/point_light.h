#ifndef POINT_LIGHT_H
#define POINT_LIGHT_H

#define MAX_POINT_LIGHTS 10

struct PointLight {
    vec3 position;
    vec3 color;
    float radius;
};

#ifdef GLSL
float point_light_attenuation(PointLight light, float distance) {
    return 1.0 / (1.0 + (2.0 / light.radius) * distance + (1.0 / (light.radius * light.radius)) * distance * distance);
}
#endif

#endif
