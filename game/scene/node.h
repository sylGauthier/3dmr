#ifndef NODE_H
#define NODE_H

#include <game/render/globject.h>
#include <game/math/linear_algebra.h>
#include <game/math/quaternion.h>

struct Node {
    struct Node** children;
    unsigned nbChildren;
    struct Node* father;

    /* Transform relative to parent node */
    Vec3 position;
    Quaternion orientation;
    Mat4 transform;
    enum NodeChangedFlags {
        NOTHING_CHANGED = 0,
        POSITION_CHANGED = 1,
        ORIENTATION_CHANGED = 2,
        PARENT_MODEL_CHANGED = 4,
        UPDATE_MODEL_MASK = (POSITION_CHANGED | ORIENTATION_CHANGED | PARENT_MODEL_CHANGED),
        UPDATE_PARENT_BB = 8
    } changedFlags;

    /* Absolute (world) transform */
    Mat4 model;
    Mat3 inverseNormal;

    Vec3 boundingBox[2];

    enum NodeType {
        NODE_EMPTY,
        NODE_GEOMETRY,
        NODE_DLIGHT,
        NODE_PLIGHT,
        NODE_CAMERA
    } type;
    union NodeData {
        struct GLObject* geometry;
        struct DirectionalLight* dlight;
        struct PointLight* plight;
        struct Camera* camera;
    } data;
};

void node_init(struct Node* node);
void node_set_geometry(struct Node* node, struct GLObject* geometry);
void node_set_dlight(struct Node* node, struct DirectionalLight* dlight);
void node_set_plight(struct Node* node, struct PointLight* plight);
void node_set_camera(struct Node* node, struct Camera* camera);

int node_add_child(struct Node* node, struct Node* child);
void nodes_free(struct Node* root, void (*free_node)(struct Node*));

int node_update_matrices(struct Node* node);
void node_update_father_bounding_box(struct Node* node);
int node_visible(const struct Node* node, const struct Camera* camera);

void node_translate(struct Node* node, const Vec3 t);
void node_rotate(struct Node* node, const Vec3 axis, float angle);
void node_rotate_q(struct Node* node, const Quaternion q);

#endif
