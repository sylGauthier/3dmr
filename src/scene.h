#include "light/light.h"
#include "node.h"

#ifndef SCENE_H
#define SCENE_H

struct Scene {
    struct Lights lights;
    struct Node root;
};

int scene_init(struct Scene* scene);
#define scene_free(scene) graph_free(&(scene)->root)

#define scene_add(scene, node) node_add_child(&(scene)->root, (node))
#define scene_render(scene, camera) render_graph(&(scene)->root, (camera), &(scene)->lights)

#endif
