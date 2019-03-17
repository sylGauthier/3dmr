#ifndef NODE_H
#define NODE_H

#include <game/render/globject.h>
#include <game/math/linear_algebra.h>
#include <game/math/quaternion.h>
#include <game/bounding_box/bounding_box.h>

struct Node {
    struct GLObject* object;

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

    int nodeLabel;
    int alwaysDraw;
    /* Bounding Box */
    struct BoundingBox bb;
};

void node_init(struct Node* node, struct GLObject* obj);
int node_add_child(struct Node* node, struct Node* child);
void node_update_matrices(struct Node* node);
int render_graph(struct Node* root, const struct Camera* cam, const struct Lights* lights);
void graph_free(struct Node* root);

void node_translate(struct Node* node, Vec3 t);
void node_rotate(struct Node* node, Vec3 axis, float angle);
void node_rotate_q(struct Node* node, Quaternion q);

#endif
