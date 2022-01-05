#include <3dmr/render/uniform_buffer.h>
#include <3dmr/render/viewer.h>
#include <3dmr/scene/node.h>

#ifndef TDMR_SCENE_H
#define TDMR_SCENE_H

struct Scene {
    struct Node root;
    struct UniformBuffer camera, lights;
    struct Node** renderQueue;
    unsigned int nRender, aRender;
};

int scene_init(struct Scene* scene, struct Camera* camera);
void scene_clean(struct Scene* scene);
void scene_free(struct Scene* scene, void (*free_node)(struct Node*));

int scene_add(struct Scene* scene, struct Node* node);
int scene_update_nodes(struct Scene* scene, void (*changedCallback)(struct Scene*, struct Node*, void*), void* userdata);
int scene_update_render_queue(struct Scene* scene, const Mat4 cameraView, const Mat4 cameraProjection);
void scene_render(struct Scene* scene, struct Lights* lights);

#endif
