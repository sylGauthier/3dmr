#include "light/light.h"

void test_init_local_light(struct LocalLight *light) {
    light->position[0] = 0;
    light->position[1] = 0;
    light->position[2] = 0;
    light->intensity = 1;
    light->decay = 0.1;
    light->ambient[0] = 0.1;
    light->ambient[1] = 0.1;
    light->ambient[2] = 0.1;
    light->diffuse[0] = 0.5;
    light->diffuse[1] = 0.5;
    light->diffuse[2] = 0.5;
    light->specular[0] = 0.5;
    light->specular[1] = 0.5;
    light->specular[2] = 0.5;
}

