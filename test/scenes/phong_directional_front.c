#include "phong.h"

int phong_directional_front_setup(struct Scene* scene) {
    scene->lights.directional[0].color[0] = 1;
    scene->lights.directional[0].color[1] = 1;
    scene->lights.directional[0].color[2] = 1;
    scene->lights.directional[0].direction[0] = 0;
    scene->lights.directional[0].direction[1] = 0;
    scene->lights.directional[0].direction[2] = -1;
    scene->lights.numDirectionalLights = 1;
    lights_buffer_object_update_dlights(&scene->lights, scene->uboLights);
    return phong_setup(scene, 1);
}

void phong_directional_front_teardown(struct Scene* scene) {
}
