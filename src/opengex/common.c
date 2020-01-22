#include <string.h>
#include <stdlib.h>

#include "opengex_common.h"

enum OgexIdentifier ogex_get_identifier(struct ODDLStructure* st) {
    if (!st->identifier)                            return OGEX_NONE;
    if (!strcmp(st->identifier, "Animation"))       return OGEX_ANIMATION;
    if (!strcmp(st->identifier, "Atten"))           return OGEX_ATTEN;
    if (!strcmp(st->identifier, "BoneCountArray"))  return OGEX_BONE_COUNT_ARRAY;
    if (!strcmp(st->identifier, "BoneIndexArray"))  return OGEX_BONE_INDEX_ARRAY;
    if (!strcmp(st->identifier, "BoneNode"))        return OGEX_BONE_NODE;
    if (!strcmp(st->identifier, "BoneRefArray"))    return OGEX_BONE_REF_ARRAY;
    if (!strcmp(st->identifier, "BoneWeightArray")) return OGEX_BONE_WEIGHT_ARRAY;
    if (!strcmp(st->identifier, "CameraNode"))      return OGEX_CAMERA_NODE;
    if (!strcmp(st->identifier, "CameraObject"))    return OGEX_CAMERA_OBJECT;
    if (!strcmp(st->identifier, "Color"))           return OGEX_COLOR;
    if (!strcmp(st->identifier, "GeometryNode"))    return OGEX_GEOMETRY_NODE;
    if (!strcmp(st->identifier, "GeometryObject"))  return OGEX_GEOMETRY_OBJECT;
    if (!strcmp(st->identifier, "IndexArray"))      return OGEX_INDEX_ARRAY;
    if (!strcmp(st->identifier, "Key"))             return OGEX_KEY;
    if (!strcmp(st->identifier, "LightNode"))       return OGEX_LIGHT_NODE;
    if (!strcmp(st->identifier, "Material"))        return OGEX_MATERIAL;
    if (!strcmp(st->identifier, "MaterialRef"))     return OGEX_MATERIAL_REF;
    if (!strcmp(st->identifier, "Mesh"))            return OGEX_MESH;
    if (!strcmp(st->identifier, "Metric"))          return OGEX_METRIC;
    if (!strcmp(st->identifier, "Morph"))           return OGEX_MORPH;
    if (!strcmp(st->identifier, "Name"))            return OGEX_NAME;
    if (!strcmp(st->identifier, "Node"))            return OGEX_NODE;
    if (!strcmp(st->identifier, "ObjectRef"))       return OGEX_OBJECT_REF;
    if (!strcmp(st->identifier, "Param"))           return OGEX_PARAM;
    if (!strcmp(st->identifier, "Rotation"))        return OGEX_ROTATION;
    if (!strcmp(st->identifier, "Scale"))           return OGEX_SCALE;
    if (!strcmp(st->identifier, "Skeleton"))        return OGEX_SKELETON;
    if (!strcmp(st->identifier, "Skin"))            return OGEX_SKIN;
    if (!strcmp(st->identifier, "Texture"))         return OGEX_TEXTURE;
    if (!strcmp(st->identifier, "Time"))            return OGEX_TIME;
    if (!strcmp(st->identifier, "Track"))           return OGEX_TRACK;
    if (!strcmp(st->identifier, "Transform"))       return OGEX_TRANSFORM;
    if (!strcmp(st->identifier, "Translation"))     return OGEX_TRANSLATION;
    if (!strcmp(st->identifier, "Value"))           return OGEX_VALUE;
    if (!strcmp(st->identifier, "VertexArray"))     return OGEX_VERTEX_ARRAY;
    return OGEX_UNKNOWN;
}

void ogex_free_context(struct OgexContext* context) {
    unsigned int i;
    for (i = 0; i < context->nbSharedObjects; i++) {
        if (!context->sharedObjs[i].persistent) {
            free(context->sharedObjs[i].object);
        }
    }
    free(context->sharedObjs);
}

int ogex_add_shared_object(struct OgexContext* context, struct ODDLStructure* oddlStruct, void* object, int persistent) {
    struct OgexSharedObject* tmp;

    if (!(tmp = realloc(context->sharedObjs, (context->nbSharedObjects + 1) * sizeof(struct OgexSharedObject)))) {
        fprintf(stderr, "Error: OpenGEX import: can't allocate memory for shared object\n");
        return 0;
    }
    context->sharedObjs = tmp;
    tmp[context->nbSharedObjects].oddlPtr = oddlStruct;
    tmp[context->nbSharedObjects].persistent = persistent;
    tmp[context->nbSharedObjects++].object = object;
    return 1;
}

void* ogex_get_shared_object(struct OgexContext* context, struct ODDLStructure* oddlStruct) {
    unsigned int i;

    for (i = 0; i < context->nbSharedObjects; i++) {
        if (context->sharedObjs[i].oddlPtr == oddlStruct) {
            return context->sharedObjs[i].object;
        }
    }
    return NULL;
}

int ogex_parse_ref(struct ODDLStructure* cur, struct ODDLStructure** res) {
    struct ODDLStructure* tmp;
    struct ODDLRef* ref;

    if (cur->nbStructures != 1) {
        fprintf(stderr, "Error: parse_object_ref: invalid nb of structures in ObjectRef\n");
        return 0;
    }
    tmp = cur->structures[0];
    if (tmp->type != TYPE_REF) {
        fprintf(stderr, "Error: parse_object_ref: invalid type in ObjectRef: %s (expected ref)\n", typeName[tmp->type]);
        return 0;
    }
    ref = tmp->dataList;
    *res = ref->ref;
    return 1;
}

int ogex_parse_param(struct ODDLStructure* cur, char** attrib, float* value) {
    struct ODDLProperty* prop;

    if (!(prop = oddl_get_property(cur, "attrib"))) {
        fprintf(stderr, "Error: Param: missing property \"attrib\"\n");
        return 0;
    }
    if (cur->nbStructures != 1) {
        fprintf(stderr, "Error: Param: invalid number of substructures\n");
        return 0;
    }
    if (cur->structures[0]->type != TYPE_FLOAT32) {
        fprintf(stderr, "Error: Param: invalid type of substructure: %s (expected FLOAT32)\n", typeName[cur->structures[0]->type]);
        return 0;
    }
    if (cur->structures[0]->nbVec != 1 || cur->structures[0]->vecSize != 1) {
        fprintf(stderr, "Error: Param: invalid data layout of substructure (expected 1 float)\n");
        return 0;
    }
    *attrib = prop->str;
    *value = *((float*)(cur->structures[0]->dataList));
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

void swap_yz(Mat4 mat) {
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

int ogex_parse_transforms(struct OgexContext* context, struct ODDLStructure* cur, Mat4* transforms, unsigned int numTransforms) {
    struct ODDLStructure* tmp;
    unsigned int i;
    Mat4* list;

    if (cur->nbStructures != 1) {
        fprintf(stderr, "Error: parse_transforms: invalid number of sub structures in Transform\n");
        return 0;
    }
    tmp = cur->structures[0];
    if (tmp->type != TYPE_FLOAT32) {
        fprintf(stderr, "Error: parse_transforms: invalid type of Transform data: %s\n", typeName[tmp->type]);
        return 0;
    }
    if (tmp->vecSize != 16 || tmp->nbVec != numTransforms) {
        fprintf(stderr, "Error: parse_transforms: invalid Transform data layout, expected float[16]\n");
        return 0;
    }
    list = tmp->dataList;
    for (i = 0; i < tmp->nbVec; i++) {
        memcpy(transforms[i], list[i], sizeof(Mat4));
        if (context->up == AXIS_Z) {
            swap_yz(transforms[i]);
        }
    }
    return 1;
}
