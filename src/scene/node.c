#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <game/scene/node.h>
#include <game/bounding_box/bounding_box.h>

static void update_bounding_box(struct Node* node, struct Node* child) {
    Vec3 bbPoints[8];
    int changed = 0, i;

    bb_compute_points(&(child->bb), bbPoints);
    for (i = 0; i < 8; i++) {
        Vec3 tmp;
        mul4m3v(tmp, child->transform, bbPoints[i]);

        changed |= bb_adapt(&(node->bb), tmp);
    }
    if (changed && node->father) {
        update_bounding_box(node->father, node);
    }
}

/* Absolutely necessary to keep transform up-to-date at all time
 * for bounding box computation.
 */
static void node_update_transform(struct Node* node) {
    quaternion_to_mat4(node->transform, node->orientation);
    memcpy(node->transform[3], node->position, sizeof(Vec3));
    if (node->father) {
        update_bounding_box(node->father, node);
    }
}

void node_init(struct Node* node, struct GLObject* obj) {
    node->object = obj;

    node->children = NULL;
    node->nbChildren = 0;
    node->father = NULL;

    node->position[0] = 0;
    node->position[1] = 0;
    node->position[2] = 0;
    quaternion_load_id(node->orientation);
    node->changedFlags = POSITION_CHANGED | ORIENTATION_CHANGED;

    if (obj && obj->vertexArray) {
        memcpy(node->bb.center, obj->vertexArray->bbCenter, sizeof(Vec3));
        memcpy(node->bb.dims, obj->vertexArray->bbDims, sizeof(Vec3));
    } else {
        memset(node->bb.center, 0, sizeof(Vec3));
        memset(node->bb.dims, 0, sizeof(Vec3));
    }

    node->nodeLabel = 0;
    node->alwaysDraw = 0;
    node_update_transform(node);
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
    update_bounding_box(node, child);
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

static int node_visible(const struct Camera* cam, const struct Node* node) {
    Vec3 points[8];
    Vec3 tmp, tmp2;
    int i;
    int upCnt = 0, leftCnt = 0, rightCnt = 0, downCnt = 0;
    int backUpCnt = 0, backDownCnt = 0, backRightCnt = 0, backLeftCnt = 0;

    bb_compute_points(&node->bb, points);

    for (i = 0; i < 8; i++) {
        mul4m3v(tmp2, node->model, points[i]);
        mul4m3v(tmp, cam->view, tmp2);
        if (tmp[2] > 0.) {
            if (tmp[0] <= 0) {
                backLeftCnt++;
            } else {
                backRightCnt++;
            }
            if (tmp[1] <= 0) {
                backDownCnt++;
            } else {
                backUpCnt++;
            }
        } else {
            mul4m3v(tmp2, cam->projection, tmp);
            if (tmp2[0] >= -1 && tmp2[0] <= 1 && tmp2[1] >= -1 && tmp2[1] <= 1 && tmp2[2] <= 0)
                return 1;
            if (tmp2[0] < -1)
                leftCnt++;
            if (tmp2[0] > 1)
                rightCnt++;
            if (tmp2[1] < -1)
                downCnt++;
            if (tmp2[1] > 1)
                upCnt++;
        }
    }
    return !(upCnt >= 8 || downCnt >= 8 || leftCnt >= 8 || rightCnt >= 8 || backRightCnt+backLeftCnt >= 8
                        || upCnt + backUpCnt >= 8
                        || leftCnt + backLeftCnt >= 8
                        || downCnt + backDownCnt >= 8
                        || rightCnt + backRightCnt >= 8);
}

int render_graph(struct Node* node, const struct Camera* cam, const struct Lights* lights) {
    unsigned int i;
    int res = 0;

    node_update_matrices(node);

    if (node->alwaysDraw || node_visible(cam, node)) {
        if (node->object) {
            globject_render(node->object, cam, lights, node->model, node->inverseNormal);
            res++;
        }
        for (i = 0; i < node->nbChildren; i++) {
            res += render_graph(node->children[i], cam, lights);
        }
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
    node_update_transform(node);
}

void node_rotate(struct Node* node, Vec3 axis, float angle) {
    Quaternion q;
    quaternion_set_axis_angle(q, axis, angle);
    node_rotate_q(node, q);
    node_update_transform(node);
}

void node_rotate_q(struct Node* node, Quaternion q) {
    Quaternion old;
    memcpy(old, node->orientation, sizeof(Quaternion));
    quaternion_mul(node->orientation, q, old);
    node->changedFlags |= ORIENTATION_CHANGED;
    node_update_transform(node);
}
