#include <game/light/light.h>
#include <game/scene/node.h>

#ifndef SCENE_H
#define SCENE_H

struct Scene {
    struct Lights lights;
    struct Node root;
    struct Camera* camera;
    GLuint uboCamera, uboLights;
};

int scene_init(struct Scene* scene, struct Camera* camera);
void scene_free(struct Scene* scene);

int scene_add(struct Scene* scene, struct Node* node);
void scene_render(struct Scene* scene);
unsigned int scene_render_count(struct Scene* scene);

#endif
