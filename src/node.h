#ifndef NODE_H
#define NODE_H

#include "geometry.h"
#include "linear_algebra.h"
#include "bounding_box.h"

struct Node {
    struct Geometry* geometry;
    struct ABoundingBox boundingBox;

    struct Node** children;
    unsigned nbChildren;

    struct Node* father;

    Mat4 transform;
};

int node_init(struct Node* node);
int node_add_child(struct Node* node, struct Node* children);
int render_graph(const struct Node* root, struct Camera* cam);

#endif
