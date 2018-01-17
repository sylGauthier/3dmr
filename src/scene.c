#include "scene.h"

int scene_init(struct Scene* scene) {
    scene->lights.numDirectional = 0;
    scene->lights.numLocal = 0;
    return node_init(&scene->root);
}
