#include "scene.h"

void scene_init(struct Scene* scene) {
    scene->lights.numDirectionalLights = 0;
    scene->lights.numPointLights = 0;
    scene->lights.ambiantLight.color[0] = 0;
    scene->lights.ambiantLight.color[1] = 0;
    scene->lights.ambiantLight.color[2] = 0;
    node_init(&scene->root, NULL);
}
