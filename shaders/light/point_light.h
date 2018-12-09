#ifndef POINT_LIGHT_H
#define POINT_LIGHT_H

#define MAX_POINT_LIGHTS 10

struct PointLight {
    vec3 position;
    vec3 color;
    float radius;
};

#endif
