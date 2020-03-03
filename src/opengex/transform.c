#include <string.h>
#include "common.h"
#include "context.h"
#include "transform.h"

static int parse_xyz_vec(const struct OgexContext* context, const struct ODDLStructure* cur, Vec3 dest) {
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
    if (!strcmp(kind, "xyz")) {
        if (!ogex_check_struct(tmp = cur->structures[0], "Transform", TYPE_FLOAT32, 1, 3)) return 0;
        memcpy(dest, tmp->dataList, sizeof(Vec3));
    } else if (*kind >= 'x' && *kind <= 'z' && !kind[1]) {
        if (!ogex_check_struct(tmp = cur->structures[0], "Transform", TYPE_FLOAT32, 1, 1)) return 0;
        zero3v(dest);
        dest[*kind - 'x'] = *(float*)(tmp->dataList);
    } else {
        fprintf(stderr, "Error: Transform: invalid kind: %s\n", kind);
        return 0;
    }
    if (context->up == AXIS_Z) ogex_swap_yz_vec(dest);
    return 1;
}

int ogex_parse_translation(const struct OgexContext* context, struct Node* node, const struct ODDLStructure* cur) {
    Vec3 translation = {0, 0, 0};

    if (!parse_xyz_vec(context, cur, translation)) {
        return 0;
    }
    node_translate(node, translation);
    return 1;
}

int ogex_parse_rotation(const struct OgexContext* context, struct Node* node, const struct ODDLStructure* cur) {
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
    ;
    if (!strcmp(kind, "axis")) {
        if (!ogex_check_struct(tmp = cur->structures[0], "Rotation", TYPE_FLOAT32, 1, 4)) return 0;
        data = tmp->dataList;
        memcpy(axis, data + 1, 3 * sizeof(float));
        angle = data[0];
    } else if (*kind >= 'x' && *kind <= 'z' && !kind[1]) {
        if (!ogex_check_struct(tmp = cur->structures[0], "Rotation", TYPE_FLOAT32, 1, 1)) return 0;
        axis[*kind - 'x'] = 1.0;
        angle = ((float*)tmp->dataList)[0];
    } else {
        fprintf(stderr, "Error: Rotation: invalid kind: %s\n", kind);
        return 0;
    }
    if (context->up == AXIS_Z) ogex_swap_yz_vec(axis);
    quaternion_set_axis_angle(quat, axis, angle);
    node_rotate_q(node, quat);
    return 1;
}

int ogex_parse_scale(const struct OgexContext* context, struct Node* node, const struct ODDLStructure* cur) {
    Vec3 scale = {1, 1, 1};

    if (!parse_xyz_vec(context, cur, scale)) {
        return 0;
    }
    node_rescale(node, scale);
    return 1;
}

int ogex_parse_transforms(const struct OgexContext* context, const struct ODDLStructure* cur, Mat4* transforms, unsigned int numTransforms) {
    struct ODDLStructure* tmp;
    unsigned int i;
    Mat4* list;

    if (cur->nbStructures != 1) {
        fprintf(stderr, "Error: parse_transforms: invalid number of sub structures in Transform\n");
        return 0;
    }
    if (!ogex_check_struct(tmp = cur->structures[0], "parse_transforms", TYPE_FLOAT32, numTransforms, 16)) return 0;
    list = tmp->dataList;
    for (i = 0; i < tmp->nbVec; i++) {
        memcpy(transforms[i], list[i], sizeof(Mat4));
        if (context->up == AXIS_Z) ogex_swap_yz_mat(transforms[i]);
    }
    return 1;
}

int extract_scale(Vec3 scale, Mat4 t) {
    scale[0] = norm3(t[0]);
    scale[1] = norm3(t[1]);
    scale[2] = norm3(t[2]);
    if (!(scale[0] && scale[1] && scale[2])) return 0;
    scale3v(t[0], 1 / scale[0]);
    scale3v(t[1], 1 / scale[1]);
    scale3v(t[2], 1 / scale[2]);
    return 1;
}
