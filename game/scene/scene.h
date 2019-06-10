#include <game/light/light.h>
#include <game/scene/node.h>

#ifndef SCENE_H
#define SCENE_H

struct Scene {
    struct Lights lights;
    struct Node root;
};

void scene_init(struct Scene* scene);
#define scene_free(scene) graph_free(&(scene)->root)

#define scene_add(scene, node) node_add_child(&(scene)->root, (node))
#define scene_render(scene, camera) render_graph(&(scene)->root, (camera), &(scene)->lights)
#define scene_render_count(scene, camera) render_graph_count(&(scene)->root, (camera), &(scene)->lights)

#endif
