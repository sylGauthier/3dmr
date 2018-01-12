#include <float.h>
#include <stdlib.h>
#include <stdio.h>

#include "node.h"

int node_init(struct Node* node) {
    struct ABoundingBox centerFull = {{0,0,0},FLT_MAX,FLT_MAX,FLT_MAX};
    node->geometry = NULL;
    node->boundingBox = centerFull;
    
    node->children = NULL;
    node->nbChildren = 0;
    node->father = NULL;

    load_id4(node->transform);
}

int node_add_child(struct Node* node, struct Node* child) {
    struct Node** tmp;

    if (!(tmp = realloc(node->children, ++(node->nbChildren)*sizeof(struct Node*)))) {
        fprintf(stderr, "Error reallocating memory for node children\n");
        node->nbChildren--;
        return 0;
    }

    node->children = tmp;
    node->children[node->nbChildren-1] = child;
    child->father = node;
    return 1;
}

static int rec_render_node(const struct Node* node, struct Camera* cam, Mat4 model) {
    Mat4 recModel;
    int i;
    int res = 1;

    mul4mm(recModel, model, node->transform);

    if (node->geometry)
        geometry_render(node->geometry, cam, recModel);

    for (i = 0; i < node->nbChildren && res; i++) {
        res = res && rec_render_node(node->children[i], cam, recModel);
    }

    return res;
}

int render_graph(const struct Node* root, struct Camera* cam) {
    Mat4 model;

    load_id4(model);

    return rec_render_node(root, cam, model);
}
