#include "phong.h"

int phong_ambient_only_setup(struct Scene* scene) {
    struct AmbientLight l = {{1, 1, 1}};
    lights_buffer_object_update_ambient(&scene->bLights, &l);
    uniform_buffer_send(&scene->bLights);
    return phong_setup(scene, 1);
}

void phong_ambient_only_teardown(struct Scene* scene) {
}
