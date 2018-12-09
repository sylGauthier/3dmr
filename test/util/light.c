#include <game/light/point_light.h>

void test_init_point_light(struct PointLight* light) {
    light->position[0] = 0;
    light->position[1] = 0;
    light->position[2] = 0;
    light->color[0] = 1.0;
    light->color[1] = 1.0;
    light->color[2] = 1.0;
    light->radius = 5.0;
}

