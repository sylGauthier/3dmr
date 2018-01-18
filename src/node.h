#ifndef NODE_H
#define NODE_H

#include "geometry/geometry.h"
#include "linear_algebra.h"
#include "quaternion.h"
#include "bounding_box.h"

struct Node {
    struct Geometry* geometry;
    struct ABoundingBox boundingBox;

    struct Node** children;
    unsigned nbChildren;

    struct Node* father;

    /* Transform relative to parent node */
    Vec3 position;
    Quaternion orientation;
    Mat4 transform;
    enum ChangedFlags {
        NOTHING_CHANGED = 0,
        POSITION_CHANGED = 1,
        ORIENTATION_CHANGED = 2,
        PARENT_MODEL_CHANGED = 4
    } changedFlags;

    /* Absolute (world) transform */
    Mat4 model;
    Mat3 inverseNormal;
};

int node_init(struct Node* node);
int node_add_child(struct Node* node, struct Node* children);
int render_graph(struct Node* root, const struct Camera* cam, const struct Lights* lights);
void graph_free(struct Node* root);

void node_translate(struct Node* node, Vec3 t);
void node_rotate(struct Node* node, Vec3 axis, float angle);
void node_rotate_q(struct Node* node, Quaternion q);

#endif
