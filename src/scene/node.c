#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <3dmr/scene/node.h>

void node_init(struct Node* node) {
    node->children = NULL;
    node->nbChildren = 0;
    node->father = NULL;

    zero3v(node->position);
    node->scale[0] = 1.0; node->scale[1] = 1.0; node->scale[2] = 1.0;
    quaternion_load_id(node->orientation);
    node->changedFlags = POSITION_CHANGED | ORIENTATION_CHANGED;
    load_id4(node->transform);
    load_id4(node->model);

    zero3v(node->boundingBox[0]);
    zero3v(node->boundingBox[1]);

    node->type = NODE_EMPTY;
    node->name = NULL;
    node->alwaysDraw = 0;
    node->hasShadow = 1;
}

static void node_reset_bounding_box(struct Node* node, Vec3 bb[2]) {
    unsigned int i;
    if (bb) {
        memcpy(node->boundingBox, bb, sizeof(node->boundingBox));
    } else {
        zero3v(node->boundingBox[0]);
        zero3v(node->boundingBox[1]);
    }
    for (i = 0; i < node->nbChildren; i++) {
        node->children[i]->changedFlags |= UPDATE_PARENT_BB;
    }
}

void node_set_geometry(struct Node* node, struct Geometry* geometry) {
    node->type = NODE_GEOMETRY;
    node->data.geometry = geometry;
    node_reset_bounding_box(node, geometry->vertexArray->boundingBox);
}

void node_set_dlight(struct Node* node, struct DirectionalLight* dlight) {
    node->type = NODE_DLIGHT;
    node->data.dlight = dlight;
    node_reset_bounding_box(node, NULL);
}

void node_set_plight(struct Node* node, struct PointLight* plight) {
    node->type = NODE_PLIGHT;
    node->data.plight = plight;
    node_reset_bounding_box(node, NULL);
}

void node_set_slight(struct Node* node, struct SpotLight* slight) {
    node->type = NODE_SLIGHT;
    node->data.slight = slight;
    node_reset_bounding_box(node, NULL);
}

void node_set_camera(struct Node* node, struct Camera* camera) {
    node->type = NODE_CAMERA;
    node->data.camera = camera;
    node_reset_bounding_box(node, NULL);
}

int node_set_name(struct Node* node, const char* name) {
    char* newname;

    if ((newname = malloc(strlen(name) + 1))) {
        strcpy(newname, name);
        node->name = newname;
        return 1;
    }
    return 0;
}

int node_add_child(struct Node* node, struct Node* child) {
    struct Node** tmp;

    if (child->father) return 0;
    if (!(tmp = realloc(node->children, ++(node->nbChildren) * sizeof(struct Node*)))) {
        node->nbChildren--;
        return 0;
    }

    node->children = tmp;
    node->children[node->nbChildren - 1] = child;
    child->father = node;
    child->changedFlags |= PARENT_MODEL_CHANGED | UPDATE_PARENT_BB;
    return 1;
}

int node_rm_child(struct Node* node, struct Node* child) {
    unsigned int i;

    if (!node->nbChildren) return 0;
    for (i = 0; i < node->nbChildren; i++) {
        if (node->children[i] == child) break;
    }
    if (i >= node->nbChildren) return 0;
    node->children[i] = node->children[node->nbChildren - 1];
    node->nbChildren--;
    child->father = NULL;
    return 1;
}

void node_free(struct Node* node) {
    free(node->children);
    free(node->name);
}

void nodes_free(struct Node* root, void (*free_node)(struct Node*)) {
    struct Node *cur, *next;

    for (cur = root; cur; cur = next) {
        if (cur->nbChildren) {
            next = cur->children[0];
        } else {
            next = (cur == root) ? NULL : cur->father;
            node_free(cur);
            if (free_node) free_node(cur);
            if (next) {
                next->children[0] = next->children[--(next->nbChildren)];
            }
        }
    }
}

static void dup_node_free(struct Node* node) {
    free(node);
}

struct Node* node_dup(struct Node* node) {
    struct Node* new = NULL;

    if (!(new = malloc(sizeof(*new)))) {
        fprintf(stderr, "Error: node_dup: malloc failed\n");
    } else {
        memcpy(new, node, sizeof(*new));
        new->children = NULL;
        new->nbChildren = 0;
        new->name = NULL;
        new->father = NULL;
        if (node->name && !(new->name = malloc(strlen(node->name) + 1))) {
            fprintf(stderr, "Error: node_dup: malloc failed\n");
        } else {
            unsigned int i;
            if (node->name) {
                strcpy(new->name, node->name);
            }
            for (i = 0; i < node->nbChildren; i++) {
                struct Node* n = NULL;
                if (!(n = node_dup(node->children[i]))) {
                    fprintf(stderr, "Error: node_dup: failed to dup child\n");
                    break;
                } else if (!node_add_child(new, n)) {
                    fprintf(stderr, "Error: node_dup: failed to add child dupped node\n");
                    nodes_free(n, dup_node_free);
                    break;
                }
            }
            if (i == node->nbChildren) return new;
        }
    }
    if (new) {
        nodes_free(new, dup_node_free);
    }
    return NULL;
}

void node_get_transform(struct Node* node, Mat4 dest) {
    quaternion_to_mat4(dest, node->orientation);
    scale3v(dest[0], node->scale[0]);
    scale3v(dest[1], node->scale[1]);
    scale3v(dest[2], node->scale[2]);
    memcpy(dest[3], node->position, sizeof(Vec3));
}

int node_update_matrices(struct Node* node) {
    unsigned int i;
    int changed;

    if (node->changedFlags & (ORIENTATION_CHANGED | SCALE_CHANGED)) {
        quaternion_to_mat4(node->transform, node->orientation);
        scale3v(node->transform[0], node->scale[0]);
        scale3v(node->transform[1], node->scale[1]);
        scale3v(node->transform[2], node->scale[2]);
    }
    if (node->changedFlags & (POSITION_CHANGED | ORIENTATION_CHANGED | SCALE_CHANGED)) {
        memcpy(node->transform[3], node->position, sizeof(Vec3));
    }
    if ((changed = (node->changedFlags & UPDATE_MODEL_MASK))) {
        if (node->father) {
            mul4mm(node->model, MAT_CONST_CAST(node->father->model), MAT_CONST_CAST(node->transform));
        } else {
            memcpy(node->model, node->transform, sizeof(Mat4));
        }
        switch (node->type) {
            case NODE_GEOMETRY:
                if (node->changedFlags & (ORIENTATION_CHANGED | PARENT_MODEL_CHANGED)) {
                    Mat3 tmp;
                    mat4to3(tmp, MAT_CONST_CAST(node->model));
                    invert3m(node->inverseNormal, MAT_CONST_CAST(tmp));
                    transpose3m(node->inverseNormal);
                }
                break;
            case NODE_DLIGHT:
                if (node->changedFlags & (ORIENTATION_CHANGED | PARENT_MODEL_CHANGED)) {
                    Mat3 tmp;
                    Vec3 down = {0, -1, 0};
                    mat4to3(tmp, MAT_CONST_CAST(node->model));
                    mul3mv(node->data.dlight->direction, MAT_CONST_CAST(tmp), down);
                }
                break;
            case NODE_PLIGHT:
                if (node->changedFlags & (POSITION_CHANGED | PARENT_MODEL_CHANGED)) {
                    mul3sv(node->data.plight->position, 1.0f / node->model[3][3], node->model[3]);
                }
                break;
            case NODE_SLIGHT:
                if (node->changedFlags & (ORIENTATION_CHANGED | PARENT_MODEL_CHANGED)) {
                    Mat3 tmp;
                    Vec3 down = {0, -1, 0};
                    mat4to3(tmp, MAT_CONST_CAST(node->model));
                    mul3mv(node->data.slight->direction, MAT_CONST_CAST(tmp), down);
                }
                if (node->changedFlags & (POSITION_CHANGED | PARENT_MODEL_CHANGED)) {
                    mul3sv(node->data.slight->position, 1.0f / node->model[3][3], node->model[3]);
                }
                break;
            case NODE_CAMERA:
                /* we need to invert the model into the camera view matrix
                 * (model = node->world, view = world->camera node)
                 * Since its only rotation+translation it's easy */
                {
                    Mat3 tmp;
                    mat4to3(tmp, MAT_CONST_CAST(node->model));
                    scale3v(tmp[0], 1. / norm3sq(tmp[0]));
                    scale3v(tmp[1], 1. / norm3sq(tmp[1]));
                    scale3v(tmp[2], 1. / norm3sq(tmp[2]));
                    transpose3m(tmp);
                    mat3to4(node->data.camera->view, MAT_CONST_CAST(tmp));
                    mul3mv(node->data.camera->view[3], MAT_CONST_CAST(tmp), node->model[3]);
                    neg3v(node->data.camera->view[3]);
                }
                break;
            default:
                break;
        }
        node->changedFlags = UPDATE_PARENT_BB;
        for (i = 0; i < node->nbChildren; i++) {
            node->children[i]->changedFlags |= PARENT_MODEL_CHANGED;
        }
    }
    return changed;
}

void node_update_father_bounding_box(struct Node* node) {
    Vec4 tmp, tmp2;
    unsigned int i, j;

    if (node->changedFlags & UPDATE_PARENT_BB) {
        if (node->father) {
            tmp[3] = 1;
            for (i = 0; i < 8; i++) {
                tmp[0] = node->boundingBox[(i >> 0) & 1][0];
                tmp[1] = node->boundingBox[(i >> 1) & 1][1];
                tmp[2] = node->boundingBox[(i >> 2) & 1][2];
                mul4mv(tmp2, MAT_CONST_CAST(node->transform), tmp);
                for (j = 0; j < 3; j++) {
                    if (node->father->boundingBox[0][j] > tmp2[j]) {
                        node->father->boundingBox[0][j] = tmp2[j];
                        node->father->changedFlags |= UPDATE_PARENT_BB;
                    }
                    if (node->father->boundingBox[1][j] < tmp2[j]) {
                        node->father->boundingBox[1][j] = tmp2[j];
                        node->father->changedFlags |= UPDATE_PARENT_BB;
                    }
                }
            }
        }
        node->changedFlags &= ~UPDATE_PARENT_BB;
    }
}

int node_visible(const struct Node* node, const Mat4 view, const Mat4 projection) {
    Vec4 tmp, tmp2;
    unsigned int i, left = 0, right = 0, down = 0, up = 0, back = 0;

    if (node->alwaysDraw) {
        return (node->alwaysDraw > 0);
    }

    tmp[3] = 1;
    for (i = 0; i < 8; i++) {
        tmp[0] = node->boundingBox[(i >> 0) & 1][0];
        tmp[1] = node->boundingBox[(i >> 1) & 1][1];
        tmp[2] = node->boundingBox[(i >> 2) & 1][2];
        mul4mv(tmp2, node->model, tmp);
        mul4mv(tmp, view, tmp2);
        if (tmp[2] > 0) {
            back++;
            left += (tmp[0] <= 0);
            right += (tmp[0] > 0);
            down += (tmp[1] <= 0);
            up += (tmp[1] > 0);
        } else {
            mul4m3v(tmp2, projection, tmp);
            if (tmp2[0] >= -1 && tmp2[0] <= 1 && tmp2[1] >= -1 && tmp2[1] <= 1 && tmp2[2] <= 0) return 1;
            left += (tmp2[0] < -1);
            right += (tmp2[0] > 1);
            down += (tmp2[1] < -1);
            up += (tmp2[1] > 1);
        }
    }
    return !(up >= 8 || down >= 8 || left >= 8 || right >= 8 || back >= 8);
}

void node_render(struct Node* n) {
    if (n->type != NODE_GEOMETRY) return;
    material_use(n->data.geometry->material);
    material_set_matrices(n->data.geometry->material, n->model, n->inverseNormal);
    vertex_array_render(n->data.geometry->vertexArray);
}

void node_translate(struct Node* node, const Vec3 t) {
    incr3v(node->position, t);
    node->changedFlags |= POSITION_CHANGED;
}

void node_shift(struct Node* node, const Vec3 t) {
    Vec4 t1;
    Vec4 t2;
    memcpy(t1, t, sizeof(Vec3));
    t1[3] = 0.0;
    mul4mv(t2, MAT_CONST_CAST(node->model), t1);
    node_translate(node, t2);
}

void node_set_scale(struct Node* node, const Vec3 scale) {
    node->scale[0] = scale[0];
    node->scale[1] = scale[1];
    node->scale[2] = scale[2];
    node->changedFlags |= SCALE_CHANGED;
}

void node_set_pos(struct Node* node, const Vec3 pos) {
    memcpy(node->position, pos, sizeof(Vec3));
    node->changedFlags |= POSITION_CHANGED;
}

void node_set_orientation(struct Node* node, Quaternion q) {
    memcpy(node->orientation, q, sizeof(Quaternion));
    node->changedFlags |= ORIENTATION_CHANGED;
}

void node_rescale(struct Node* node, const Vec3 s) {
    node->scale[0] *= s[0];
    node->scale[1] *= s[1];
    node->scale[2] *= s[2];
    node->changedFlags |= SCALE_CHANGED;
}

void node_rotate(struct Node* node, const Vec3 axis, float angle) {
    Quaternion q;
    quaternion_set_axis_angle(q, axis, angle);
    node_rotate_q(node, q);
}

void node_slew(struct Node* node, const Vec3 axis, float angle) {
    Vec3 newAxis;
    quaternion_compose(newAxis, node->orientation, axis);
    node_rotate(node, newAxis, angle);
}

void node_rotate_q(struct Node* node, const Quaternion q) {
    Quaternion old;
    memcpy(old, node->orientation, sizeof(Quaternion));
    quaternion_mul(node->orientation, q, old);
    node->changedFlags |= ORIENTATION_CHANGED;
}
