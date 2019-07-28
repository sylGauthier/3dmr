#include "phong.h"

int phong_directional_back_setup(struct Scene* scene) {
    struct DirectionalLight l = {{0, 0, 1}, {1, 1, 1}};
    lights_buffer_object_update_dlight(&scene->lights, &l, 0);
    lights_buffer_object_update_ndlight(&scene->lights, 1);
    uniform_buffer_send(&scene->lights);
    return phong_setup(scene, 0);
}

void phong_directional_back_teardown(struct Scene* scene) {
}
