#include "phong.h"

int phong_ambient_only_setup(struct Scene* scene) {
    scene->lights.ambientLight.color[0] = 1;
    scene->lights.ambientLight.color[1] = 1;
    scene->lights.ambientLight.color[2] = 1;
    lights_buffer_object_update_ambient(&scene->lights.ambientLight, scene->uboLights);
    return phong_setup(scene, 1);
}

void phong_ambient_only_teardown(struct Scene* scene) {
}
