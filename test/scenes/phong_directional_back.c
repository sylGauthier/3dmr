#include "phong.h"

int phong_directional_back_setup(struct Scene* scene) {
    struct DirectionalLight l = {{0, 0, 1}, {1, 1, 1}};
    lights_buffer_object_update_dlight(&scene->bLights, &l, 0);
    lights_buffer_object_update_ndlight(&scene->bLights, 1);
    uniform_buffer_send(&scene->bLights);
    return phong_setup(scene, 0);
}

void phong_directional_back_teardown(struct Scene* scene) {
}
