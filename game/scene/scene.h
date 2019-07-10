#include <game/light/light.h>
#include <game/scene/node.h>

#ifndef SCENE_H
#define SCENE_H

struct Scene {
    struct Lights lights;
    struct Node root;
    struct Camera* camera;
    GLuint uboCamera, uboLights;
    struct Node** renderQueue;
    unsigned int nRender, aRender;
};

int scene_init(struct Scene* scene, struct Camera* camera);
void scene_free(struct Scene* scene, void (*free_node)(struct Node*));

int scene_add(struct Scene* scene, struct Node* node);
int scene_update_nodes(struct Scene* scene, void (*changedCallback)(struct Node*));
int scene_update_render_queue(struct Scene* scene);
void scene_render(struct Scene* scene);

#endif
