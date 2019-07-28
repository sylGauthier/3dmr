#include <game/render/uniform_buffer.h>
#include <game/render/viewer.h>
#include <game/scene/node.h>

#ifndef SCENE_H
#define SCENE_H

struct Scene {
    struct Node root;
    struct UniformBuffer camera, lights;
    struct Node** renderQueue;
    unsigned int nRender, aRender;
};

int scene_init(struct Scene* scene, struct Camera* camera);
void scene_free(struct Scene* scene, void (*free_node)(struct Node*));

int scene_add(struct Scene* scene, struct Node* node);
int scene_update_nodes(struct Scene* scene, void (*changedCallback)(struct Scene*, struct Node*, void*), void* userdata);
int scene_update_render_queue(struct Scene* scene, const Mat4 cameraView, const Mat4 cameraProjection);
void scene_render(struct Scene* scene);

#endif
