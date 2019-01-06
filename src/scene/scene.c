#include <game/scene/scene.h>

void scene_init(struct Scene* scene) {
    light_init(&scene->lights);
    node_init(&scene->root, NULL);
}
