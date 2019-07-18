#include "pbr.h"

int pbr_no_light_setup(struct Scene* scene) {
    return pbr_setup(scene);
}

void pbr_no_light_teardown(struct Scene* scene) {
}
