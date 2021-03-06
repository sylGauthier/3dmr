#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "context.h"
#include "skin.h"
#include "transform.h"

static int parse_bone_ref_array(const struct OgexContext* context, const struct ODDLStructure* cur, struct Node** bones, unsigned int numBones) {
    struct ODDLStructure* tmp;
    struct ODDLRef* refs;
    unsigned int i;

    if (cur->nbStructures != 1) {
        fprintf(stderr, "Error: BoneRefArray: must contain exactly one substructure\n");
        return 0;
    }
    if (!ogex_check_struct(tmp = cur->structures[0], "BoneRefArray", TYPE_REF, numBones, 1)) return 0;
    refs = tmp->dataList;
    for (i = 0; i < numBones; i++) {
        struct ODDLStructure* ref = refs[i].ref;
        if (!ref) {
            fprintf(stderr, "Error: BoneRefArray: bone ref can't be null\n");
            return 0;
        }
        if (!ref->identifier || strcmp(ref->identifier, "BoneNode")) {
            fprintf(stderr, "Error: BoneRefArray: a ref points to an object that isn't a bone\n");
            return 0;
        }
        if (!(bones[i] = ogex_context_find_object(&context->boneNodes, ref))) {
            fprintf(stderr, "Error: BoneRefArray: a ref points to a bone node that doesn't exist, make sure the skeleton is declared after all bone nodes\n");
            return 0;
        }
    }
    return 1;
}

static int parse_bind_pose(const struct OgexContext* context, const struct ODDLStructure* cur, Mat4* transforms, unsigned int numTransforms) {
    return ogex_parse_transforms(context, cur, transforms, numTransforms);
}

static int parse_skeleton(const struct OgexContext* context, const struct ODDLStructure* cur, struct Skin* skin) {
    unsigned int i;

    for (i = 0; i < cur->nbStructures; i++) {
        struct ODDLStructure* tmp = cur->structures[i];

        if (!tmp->identifier) {
            continue;
        } else if (!strcmp(tmp->identifier, "BoneRefArray")) {
            if (!parse_bone_ref_array(context, tmp, skin->bones, skin->numBones)) return 0;
        } else if (!strcmp(tmp->identifier, "Transform")) {
            if (!parse_bind_pose(context, tmp, skin->bindPose, skin->numBones)) return 0;
        }
    }
    return 1;
}

static unsigned int skeleton_num_bones(const struct ODDLStructure* cur) {
    unsigned int i, *n, numBones = 0, numTransforms = 0;

    if (cur->nbStructures != 2) {
        fprintf(stderr, "Error: Skeleton: must contain one BoneRefArray and one Transform\n");
        return 0;
    }
    for (i = 0; i < cur->nbStructures; i++) {
        struct ODDLStructure* tmp = cur->structures[i];

        if (!tmp->identifier) {
            continue;
        } else if (!strcmp(tmp->identifier, "BoneRefArray")) {
            n = &numBones;
        } else if (!strcmp(tmp->identifier, "Transform")) {
            n = &numTransforms;
        } else {
            continue;
        }
        if (tmp->nbStructures != 1) {
            fprintf(stderr, "Error: BoneRefArray/Transform: must contain exactly one substructure\n");
        } else if (*n) {
            fprintf(stderr, "Error: Skeleton: must contain only one BoneRefArray/Transform\n");
        } else if (!(*n = tmp->structures[0]->nbVec)) {
            fprintf(stderr, "Error: BoneRefArray/Transform: need at least one\n");
        } else {
            continue;
        }
        return 0;
    }
    if (numBones != numTransforms) {
        fprintf(stderr, "Error: Skeleton: inconsistent number of bind pose transforms\n");
        return 0;
    }
    return numBones;
}

static int parse_int_array(const struct ODDLStructure* cur, unsigned int** array, unsigned int* len) {
    struct ODDLStructure* tmp;
    unsigned int i;
    uint8_t* u8;
    uint16_t* u16;
    uint32_t* u32;
    uint64_t* u64;

    if (cur->nbStructures != 1) {
        fprintf(stderr, "Error: Skin: Int arrays must have exactly one sub structure\n");
        return 0;
    }
    tmp = cur->structures[0];
    if (!(tmp->type == TYPE_UINT8 || tmp->type == TYPE_UINT16 || tmp->type == TYPE_UINT32 || tmp->type == TYPE_UINT64)
     || tmp->vecSize != 1) {
        fprintf(stderr, "Error: Skin: Int arrays must be of an unsigned integer type with only single value vectors\n");
        printf("type=%d, vecSize=%d\n", tmp->type, tmp->vecSize);
        return 0;
    }
    if (!(*array = malloc(tmp->nbVec * sizeof(unsigned int)))) {
        fprintf(stderr, "Error: Skin: could not allocate memory for int array\n");
        return 0;
    }
    u8 = tmp->dataList;
    u16 = tmp->dataList;
    u32 = tmp->dataList;
    u64 = tmp->dataList;
    for (i = 0; i < tmp->nbVec; i++) {
        switch (tmp->type) {
            case TYPE_UINT8: (*array)[i] = u8[i]; break;
            case TYPE_UINT16: (*array)[i] = u16[i]; break;
            case TYPE_UINT32: (*array)[i] = u32[i]; break;
            case TYPE_UINT64: (*array)[i] = u64[i]; break;
            default:
                fprintf(stderr, "Error: Skin: this program has reached a state that I thought was unreachable. I'm sorry I failed you.\n");
                free(*array);
                return 0;
        }
    }
    *len = tmp->nbVec;
    return 1;
}

static int parse_float_array(const struct ODDLStructure* cur, float** array, unsigned int* len) {
    struct ODDLStructure* tmp;

    if (cur->nbStructures != 1) {
        fprintf(stderr, "Error: Skin: Float arrays must have exactly one sub structure\n");
        return 0;
    }
    tmp = cur->structures[0];
    if (tmp->type != TYPE_FLOAT32 || tmp->vecSize != 1) {
        fprintf(stderr, "Error: Skin: Float arrays must be of type float with only single value vectors\n");
        return 0;
    }
    if (!(*array = malloc(tmp->nbVec * sizeof(float)))) {
        fprintf(stderr, "Error: Skin: could not allocate memory for float array\n");
        return 0;
    }
    memcpy(*array, tmp->dataList, tmp->nbVec * sizeof(float));
    *len = tmp->nbVec;
    return 1;
}

static int load_skin_arrays(const unsigned int* countArray, unsigned int countLen, const unsigned int* indexArray, unsigned int idxLen, const float* weightArray, unsigned int weightLen, unsigned int* indices, float* weights) {
    unsigned int cntPos;
    unsigned int idxPos = 0;

    for (cntPos = 0; cntPos < countLen; cntPos++) {
        unsigned int offset;
        unsigned int topIndices[2] = {0};
        float topWeights[2] = {0};
        /* We're using only 2 bones per vertex, so we select the ones with highest weight */
        for (offset = 0; offset < countArray[cntPos]; offset++) {
            if (idxPos + offset >= idxLen || idxPos + offset >= weightLen) {
                fprintf(stderr, "Error: Skin: BoneIndexArray or BoneWeightArray is too short\n");
                return 0;
            }
            if (weightArray[idxPos + offset] > topWeights[1]) {
                if (weightArray[idxPos + offset] > topWeights[0]) {
                    topWeights[1] = topWeights[0];
                    topWeights[0] = weightArray[idxPos + offset];
                    topIndices[1] = topIndices[0];
                    topIndices[0] = indexArray[idxPos + offset];
                } else {
                    topWeights[1] = weightArray[idxPos + offset];
                    topIndices[1] = indexArray[idxPos + offset];
                }
            }
        }
        indices[2 * cntPos] = topIndices[0];
        indices[2 * cntPos + 1] = topIndices[1];
        weights[2 * cntPos] = topWeights[0];
        weights[2 * cntPos + 1] = topWeights[1];
        idxPos += offset;
    }
    return 1;
}

int ogex_post_parse_skeletons(struct OgexContext* context) {
    unsigned int i;
    for ( i = 0; i < context->skins.num; i++) {
        if (!parse_skeleton(context, context->skins.structures[i], context->skins.objects[i])) return 0;
    }
    return 1;
}

static int check_existing(const void* thing, const char* name) {
    if (thing) {
        fprintf(stderr, "Error: Skin: only one %s is allowed\n", name);
        return 0;
    }
    return 1;
}

int ogex_parse_skin(struct OgexContext* context, struct ODDLStructure* cur, struct Skin** skin, unsigned int** indices, float** weights, unsigned int* numVertices) {
    Mat4 skinTransform;
    unsigned int i, countLen = 0, idxLen = 0, weightLen = 0, numBones = 0;
    unsigned int* countArray = NULL;
    unsigned int* indexArray = NULL;
    const struct ODDLStructure* skeleton = NULL;
    float* weightArray = NULL;
    int success = 1;

    load_id4(skinTransform);
    for (i = 0; i < cur->nbStructures && success; i++) {
        struct ODDLStructure* tmp = cur->structures[i];

        if (!tmp->identifier) {
            continue;
        } else if (!strcmp(tmp->identifier, "Transform")) {
            success = ogex_parse_transforms(context, tmp, &skinTransform, 1);
        } else if (!strcmp(tmp->identifier, "Skeleton")) {
            success = check_existing(skeleton, "Skeleton") && (numBones = skeleton_num_bones(skeleton = tmp));
        } else if (!strcmp(tmp->identifier, "BoneCountArray")) {
            success = check_existing(countArray, "BoneCountArray") && parse_int_array(tmp, &countArray, &countLen);
        } else if (!strcmp(tmp->identifier, "BoneIndexArray")) {
            success = check_existing(indexArray, "BoneIndexArray") && parse_int_array(tmp, &indexArray, &idxLen);
        } else if (!strcmp(tmp->identifier, "BoneWeightArray")) {
            success = check_existing(weightArray, "BoneWeightArray") && parse_float_array(tmp, &weightArray, &weightLen);
        }
    }
    if (success) {
        success = 0;
        if (!skeleton || !countArray || !indexArray || !weightArray) {
            fprintf(stderr, "Error: Skin: missing substructure (Skeleton, BoneCountArray, BoneIndexArray, BoneWeightArray)\n");
        } else if (!(*skin = skin_new(numBones))) {
            fprintf(stderr, "Error: Skin: failed to allocate skin\n");
        } else {
            *indices = malloc(2 * countLen * sizeof(**indices));
            *weights = malloc(2 * countLen * sizeof(**weights));
            *numVertices = countLen;
            if (!*indices || !*weights) {
                fprintf(stderr, "Error: Skin: couldn't allocate memory for index or weight array\n");
            } else {
                load_skin_arrays(countArray, countLen, indexArray, idxLen, weightArray, weightLen, *indices, *weights);
                memcpy((*skin)->skinTransform, skinTransform, sizeof(Mat4));
                if (!(success = ogex_context_add_object(&context->skins, skeleton, *skin))) {
                    fprintf(stderr, "Error: Skin: memory allocation failed\n");
                }
            }
            if (!success) {
                free(*indices);
                free(*weights);
                skin_free(*skin);
            }
        }
    }
    free(countArray);
    free(indexArray);
    free(weightArray);
    return success;
}
