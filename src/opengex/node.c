#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <game/material/phong.h>

#include "opengex_common.h"

static int extract_scale(Vec3 scale, Mat4 t) {
    scale[0] = norm3(t[0]);
    scale[1] = norm3(t[1]);
    scale[2] = norm3(t[2]);
    if (!(scale[0] && scale[1] && scale[2])) return 0;
    scale3v(t[0], 1 / scale[0]);
    scale3v(t[1], 1 / scale[1]);
    scale3v(t[2], 1 / scale[2]);
    return 1;
}

static void swap_yz(Mat4 mat) {
    Vec4 tmp;
    unsigned int i;

    /* Swap columns 1 and 2 */
    memcpy(tmp, mat[1], sizeof(Vec4));
    memcpy(mat[1], mat[2], sizeof(Vec4));
    memcpy(mat[2], tmp, sizeof(Vec4));
    /* Swap rows 1 and 2 */
    for (i = 0; i < 4; i++) {
        float tmpf = mat[i][1];
        mat[i][1] = mat[i][2];
        mat[i][2] = tmpf;
    }
    /* Negate relevant coefs */
    mat[0][2] *= -1.0;
    mat[1][2] *= -1.0;
    mat[2][0] *= -1.0;
    mat[2][1] *= -1.0;
    mat[3][2] *= -1.0;
}

static int parse_transform(struct OgexContext* context, struct Node* node, struct ODDLStructure* cur) {
    struct ODDLStructure* tmp;
    Mat4 transform;
    Vec3 scale;
    Quaternion quat;

    if (cur->nbStructures != 1) {
        fprintf(stderr, "Error: parse_transform: invalid number of sub structures in Transform\n");
        return 0;
    }
    tmp = cur->structures[0];
    if (tmp->type != TYPE_FLOAT32) {
        fprintf(stderr, "Error: parse_transform: invalid type of Transform data: %s\n", typeName[tmp->type]);
        return 0;
    }
    if (tmp->vecSize != 16 || tmp->nbVec != 1) {
        fprintf(stderr, "Error: parse_transform: invalid Transform data layout, expected float[16]\n");
        return 0;
    }
    memcpy(transform, tmp->dataList, sizeof(Mat4));
    if (context->up == AXIS_Z) {
        swap_yz(transform);
    }
    if (!extract_scale(scale, transform)) {
        fprintf(stderr, "Error: parse_transform: invalid transform matrix (null scaling)\n");
        return 0;
    }
    quaternion_from_mat4(quat, MAT_CONST_CAST(transform));
    node_rescale(node, scale);
    node_rotate_q(node, quat);
    node_translate(node, transform[3]);
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

    for (i = 0; i < cur->nbStructures; i++) {
        struct ODDLStructure* tmp = cur->structures[i];

        switch (ogex_get_identifier(tmp)) {
            case OGEX_NAME:
                break;
            case OGEX_TRANSFORM:
                parse_transform(context, newNode, tmp);
                break;
            case OGEX_TRANSLATION:
                if (!parse_translation(context, newNode, tmp)) {
                    free(newNode);
                    return 0;
                }
                break;
            case OGEX_ROTATION:
                if (!parse_rotation(context, newNode, tmp)) {
                    free(newNode);
                    return 0;
                }
                break;
            case OGEX_SCALE:
                if (!parse_scale(context, newNode, tmp)) {
                    free(newNode);
                    return 0;
                }
                break;
            case OGEX_ANIMATION:
                if (!ogex_parse_animation(context, newNode, tmp)) {
                    free(newNode);
                    return 0;
                }
                break;
            case OGEX_NODE:
            case OGEX_BONE_NODE:
            case OGEX_GEOMETRY_NODE:
            case OGEX_CAMERA_NODE:
            case OGEX_LIGHT_NODE:
                if (!ogex_parse_node(context, newNode, tmp)) {
                    free(newNode);
                    return 0;
                }
            default:
                break;
        }
    }

    switch (ogex_get_identifier(cur)) {
        case OGEX_BONE_NODE:
            fprintf(stderr, "Warning: BoneNode: not implemented\n");
            break;
        case OGEX_GEOMETRY_NODE:
            if (!ogex_parse_geometry_node(context, newNode, cur)) {
                free(newNode);
                return 0;
            }
            break;
        case OGEX_CAMERA_NODE:
            if (!ogex_parse_camera_node(context, newNode, cur)) {
                free(newNode);
                return 0;
            }
            break;
        case OGEX_LIGHT_NODE:
            if (!ogex_parse_light_node(context, newNode, cur)) {
                free(newNode);
                return 0;
            }
            break;
        default:
            break;
    }
    return node_add_child(root, newNode);
}
