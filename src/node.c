#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>

#include "node.h"

void node_init(struct Node* node, struct Geometry* geometry) {
    struct ABoundingBox centerFull = {{0, 0, 0}, FLT_MAX, FLT_MAX, FLT_MAX};
    node->geometry = geometry;
    node->boundingBox = centerFull;

    node->children = NULL;
    node->nbChildren = 0;
    node->father = NULL;

    node->position[0] = 0;
    node->position[1] = 0;
    node->position[2] = 0;
    quaternion_load_id(node->orientation);
    node->changedFlags = POSITION_CHANGED | ORIENTATION_CHANGED;
}

int node_add_child(struct Node* node, struct Node* child) {
    struct Node** tmp;

    if (!(tmp = realloc(node->children, ++(node->nbChildren) * sizeof(struct Node*)))) {
        fprintf(stderr, "Error reallocating memory for node children\n");
        node->nbChildren--;
        return 0;
    }

    node->children = tmp;
    node->children[node->nbChildren - 1] = child;
    child->father = node;
    node->changedFlags |= PARENT_MODEL_CHANGED;
    return 1;
}

void node_update_matrices(struct Node* node) {
    unsigned int i;
    enum ChangedFlags modelChanged = NOTHING_CHANGED;

    if (node->changedFlags & ORIENTATION_CHANGED) {
        quaternion_to_mat4(node->transform, node->orientation);
    }
    if (node->changedFlags & (POSITION_CHANGED | ORIENTATION_CHANGED)) {
        memcpy(node->transform[3], node->position, sizeof(Vec3));
    }
    if (node->changedFlags) {
        if (node->father) {
            mul4mm(node->model, node->father->model, node->transform);
        } else {
            memcpy(node->model, node->transform, sizeof(Mat4));
        }
        modelChanged = PARENT_MODEL_CHANGED;
    }
    if (node->changedFlags & (ORIENTATION_CHANGED | PARENT_MODEL_CHANGED)) {
        Mat3 tmp;
        mat4to3(tmp, node->model);
        invert3m(node->inverseNormal, tmp);
        transpose3m(node->inverseNormal);
    }

    for (i = 0; i < node->nbChildren; i++) {
        node->children[i]->changedFlags |= modelChanged;
    }

    node->changedFlags = NOTHING_CHANGED;
}

int render_graph(struct Node* node, const struct Camera* cam, const struct Lights* lights) {
    unsigned int i;
    int res = 1;

    node_update_matrices(node);

    if (node->geometry) {
        geometry_render(node->geometry, cam, lights, node->model, node->inverseNormal);
    }

    for (i = 0; i < node->nbChildren && res; i++) {
        res = res && render_graph(node->children[i], cam, lights);
    }

    return res;
}

void graph_free(struct Node* root) {
    int i;

    for (i = 0; i < root->nbChildren; i++) {
        graph_free(root->children[i]);
    }
    free(root->children);
}

void node_translate(struct Node* node, Vec3 t) {
    incr3v(node->position, t);
    node->changedFlags |= POSITION_CHANGED;
}

void node_rotate(struct Node* node, Vec3 axis, float angle) {
    Quaternion q;
    quaternion_set_axis_angle(q, axis, angle);
    node_rotate_q(node, q);
}

void node_rotate_q(struct Node* node, Quaternion q) {
    Quaternion old;
    memcpy(old, node->orientation, sizeof(Quaternion));
    quaternion_mul(node->orientation, q, old);
    node->changedFlags |= ORIENTATION_CHANGED;
}
