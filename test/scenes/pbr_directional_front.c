#include "pbr.h"

int pbr_directional_front_setup(struct Scene* scene) {
    struct DirectionalLight l = {{0, 0, -1}, {1, 1, 1}};
    lights_buffer_object_update_dlight(&scene->lights, &l, 0);
    lights_buffer_object_update_ndlight(&scene->lights, 1);
    uniform_buffer_send(&scene->lights);
    return pbr_setup(scene);
}

void pbr_directional_front_teardown(struct Scene* scene) {
}
