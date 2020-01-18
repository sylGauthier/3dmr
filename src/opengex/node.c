#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <game/material/phong.h>

#include "opengex_common.h"

static int parse_node_transform(struct OgexContext* context, struct Node* node, struct ODDLStructure* cur) {
    Mat4 transform;
    Vec3 scale;
    Quaternion quat;

    if (!ogex_parse_transform(context, cur, transform)) {
        return 0;
    }
    if (!extract_scale(scale, transform)) {
        fprintf(stderr, "Error: parse_node_transform: invalid transform matrix (null scaling)\n");
        return 0;
    }
    quaternion_from_mat4(quat, MAT_CONST_CAST(transform));
    node_set_scale(node, scale);
    node_set_orientation(node, quat);
    node_set_pos(node, transform[3]);
    return 1;
}

static int parse_xyz_vec(const struct OgexContext* context, struct ODDLStructure* cur, Vec3 dest) {
    struct ODDLStructure* tmp;
    char* kind;
    struct ODDLProperty* prop;

    if (!(prop = oddl_get_property(cur, "kind"))) {
        kind = "xyz";
    } else {
        kind = prop->str;
    }
    if (cur->nbStructures != 1) {
        fprintf(stderr, "Error: Transform: invalid number of substructures\n");
        return 0;
    }
    tmp = cur->structures[0];
    if (tmp->type != TYPE_FLOAT32) {
        fprintf(stderr, "Error: Transform: invalid data type: %s\n", typeName[tmp->type]);
        return 0;
    }
    if (!strcmp(kind, "xyz")) {
        if (tmp->vecSize != 3 || tmp->nbVec != 1) {
            fprintf(stderr, "Error: Transform: invalid data layout\n");
            return 0;
        }
        memcpy(dest, tmp->dataList, sizeof(Vec3));
    } else if (*kind >= 'x' && *kind <= 'z' && !kind[1]) {
        if (tmp->vecSize != 1 || tmp->nbVec != 1) {
            fprintf(stderr, "Error: Transform: invalid data layout\n");
            return 0;
        }
        dest[*kind - 'x'] = *(float*)(tmp->dataList);
    } else {
        fprintf(stderr, "Error: Transform: invalid kind: %s\n", kind);
        return 0;
    }
    if (context->up == AXIS_Z) {
        float tmpF;
        tmpF = dest[1];
        dest[1] = dest[2];
        dest[2] = -tmpF;
    }
    return 1;
}

static int parse_translation(struct OgexContext* context, struct Node* node, struct ODDLStructure* cur) {
    Vec3 translation = {0, 0, 0};

    if (!parse_xyz_vec(context, cur, translation)) {
        return 0;
    }
    node_translate(node, translation);
    return 1;
}

static int parse_rotation(struct OgexContext* context, struct Node* node, struct ODDLStructure* cur) {
    struct ODDLStructure* tmp;
    char* kind;
    struct ODDLProperty* prop;
    Quaternion quat;
    Vec3 axis = {0};
    float angle;
    float* data;

    quaternion_load_id(quat);

    if (!(prop = oddl_get_property(cur, "kind"))) {
        kind = "axis";
    } else {
        kind = prop->str;
    }
    if (cur->nbStructures != 1) {
        fprintf(stderr, "Error: Rotation: invalid number of substructures\n");
        return 0;
    }
    tmp = cur->structures[0];
    if (tmp->type != TYPE_FLOAT32) {
        fprintf(stderr, "Error: Rotation: invalid type of Translation data: %s\n", typeName[tmp->type]);
        return 0;
    }
    data = tmp->dataList;
    if (!strcmp(kind, "axis")) {
        if (tmp->vecSize != 4 || tmp->nbVec != 1) {
            fprintf(stderr, "Error: Rotation: invalid data layout\n");
            return 0;
        }
        memcpy(axis, data + 1, 3 * sizeof(float));
        angle = data[0];
    } else if (*kind >= 'x' && *kind <= 'z' && !kind[1]) {
        if (tmp->vecSize != 1 || tmp->nbVec != 1) {
            fprintf(stderr, "Error: Rotation: invalid data layout\n");
            return 0;
        }
        axis[*kind - 'x'] = 1.0;
        angle = data[0];
    } else {
        fprintf(stderr, "Error: Rotation: invalid kind: %s\n", kind);
        return 0;
    }
    if (context->up == AXIS_Z) {
        float tmpF;
        tmpF = axis[1];
        axis[1] = axis[2];
        axis[2] = -tmpF;
    }
    quaternion_set_axis_angle(quat, axis, angle);
    node_rotate_q(node, quat);
    return 1;
}

static int parse_scale(struct OgexContext* context, struct Node* node, struct ODDLStructure* cur) {
    Vec3 scale = {1, 1, 1};

    if (!parse_xyz_vec(context, cur, scale)) {
        return 0;
    }
    node_rescale(node, scale);
    return 1;
}

int ogex_parse_node(struct OgexContext* context, struct Node* root, struct ODDLStructure* cur) {
    unsigned int i;
    struct Node* newNode;

    if (!(cur->structures)) return 0;

    if (!(newNode = malloc(sizeof(*newNode)))) {
        fprintf(stderr, "Error: Node: can't allocate memory for new node\n");
        return 0;
    }
    node_init(newNode);
    if (!node_add_child(root, newNode)) {
        free(newNode);
        return 0;
    }

    for (i = 0; i < cur->nbStructures; i++) {
        struct ODDLStructure* tmp = cur->structures[i];

        switch (ogex_get_identifier(tmp)) {
            case OGEX_NAME:
                break;
            case OGEX_TRANSFORM:
                parse_node_transform(context, newNode, tmp);
                node_update_matrices(newNode);
                break;
            case OGEX_TRANSLATION:
                if (!parse_translation(context, newNode, tmp)) {
                    return 0;
                }
                break;
            case OGEX_ROTATION:
                if (!parse_rotation(context, newNode, tmp)) {
                    return 0;
                }
                break;
            case OGEX_SCALE:
                if (!parse_scale(context, newNode, tmp)) {
                    return 0;
                }
                break;
            case OGEX_ANIMATION:
                if (!ogex_parse_animation(context, newNode, tmp)) {
                    return 0;
                }
                break;
            case OGEX_NODE:
            case OGEX_BONE_NODE:
            case OGEX_GEOMETRY_NODE:
            case OGEX_CAMERA_NODE:
            case OGEX_LIGHT_NODE:
                if (!ogex_parse_node(context, newNode, tmp)) {
                    return 0;
                }
            default:
                break;
        }
    }

    switch (ogex_get_identifier(cur)) {
        case OGEX_BONE_NODE:
            newNode->type = NODE_BONE;
            ogex_add_shared_object(context, cur, newNode, 1);
            break;
        case OGEX_GEOMETRY_NODE:
            if (!ogex_parse_geometry_node(context, newNode, cur)) {
                return 0;
            }
            break;
        case OGEX_CAMERA_NODE:
            if (!ogex_parse_camera_node(context, newNode, cur)) {
                return 0;
            }
            break;
        case OGEX_LIGHT_NODE:
            if (!ogex_parse_light_node(context, newNode, cur)) {
                return 0;
            }
            break;
        default:
            break;
    }
    return 1;
}
