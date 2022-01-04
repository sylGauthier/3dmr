#ifndef DIRECTIONAL_LIGHT_H
#define DIRECTIONAL_LIGHT_H

#define MAX_DIRECTIONAL_LIGHTS 10

struct DirectionalLight {
    vec3 direction;
    vec3 color;

    bool shadow;
    int depthMapID;
    mat4 projection;
    mat4 view;
};

#endif
