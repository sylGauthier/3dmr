#include "phong.h"

int phong_no_light_setup(struct Scene* scene) {
    return phong_setup(scene, 1);
}

void phong_no_light_teardown(struct Scene* scene) {
}
