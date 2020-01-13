#include <stdlib.h>
#include <string.h>

#include "opengex_common.h"

static int parse_bone_ref_array(struct OgexContext* context, struct ODDLStructure* cur, struct Skin* skin) {
    struct ODDLStructure* tmp;
    struct ODDLRef* refs;
    unsigned int i;

    if (skin->bones) {
        fprintf(stderr, "Error: BoneRefArray: Skin already has an array of bones?\n");
        return 0;
    }
    if (cur->nbStructures != 1) {
        fprintf(stderr, "Error: BoneRefArray: must contain exactly one substructure\n");
        return 0;
    }
    tmp = cur->structures[0];
    if (tmp->type != TYPE_REF || tmp->vecSize != 1) {
        fprintf(stderr, "Error: BoneRefArray: substructure must be an array of single refs\n");
        return 0;
    }
    if (skin->nbBones && tmp->nbVec != skin->nbBones) {
        fprintf(stderr, "Error: BoneRefArray: inconsistent number of bones\n");
        return 0;
    }
    if (!(skin->bones = malloc(tmp->nbVec * sizeof(void*)))) {
        fprintf(stderr, "Error: BoneRefArray: could not allocate memory for bone array\n");
        return 0;
    }
    refs = tmp->dataList;
    for (i = 0; i < tmp->nbVec; i++) {
        struct ODDLStructure* ref = refs[i].ref;
        if (!(ogex_get_identifier(ref) == OGEX_BONE_NODE)) {
            fprintf(stderr, "Error: BoneRefArray: a ref points to an object that isn't a bone\n");
            free(skin->bones);
            return 0;
        }
        if (!(skin->bones[i] = ogex_get_shared_object(context, ref))) {
            fprintf(stderr, "Error: BoneRefArray: a ref points to a bone node that doesn't exist, make sure "
                            "the skeleton is declared after all bone nodes\n");
            free(skin->bones);
            return 0;
        }
    }
    skin->nbBones = tmp->nbVec;
    return 1;
}

static int parse_bind_pose(struct OgexContext* context, struct ODDLStructure* cur, struct Skin* skin) {
    Mat4* tmp;
    unsigned int nbTransforms;

    if (!(ogex_parse_transforms(context, cur, &tmp, &nbTransforms))) {
        return 0;
    }
    if (skin->nbBones && nbTransforms != skin->nbBones) {
        fprintf(stderr, "Error: Skeleton: inconsistent number of bind pose transforms (nbBones=%d, nbTransforms=%d\n",
                skin->nbBones, nbTransforms);
        free(tmp);
        return 0;
    }
    skin->nbBones = nbTransforms;
    skin->bindPose = tmp;
    return 1;
}

static int parse_skeleton(struct OgexContext* context, struct ODDLStructure* cur, struct Skin* skin) {
    unsigned int i;

    for (i = 0; i < cur->nbStructures; i++) {
        struct ODDLStructure* tmp = cur->structures[i];

        switch (ogex_get_identifier(tmp)) {
            case OGEX_BONE_REF_ARRAY:
                if (!parse_bone_ref_array(context, tmp, skin)) {
                    return 0;
                }
                break;
            case OGEX_TRANSFORM:
                if (!parse_bind_pose(context, tmp, skin)) {
                    return 0;
                }
                break;
            default:
                break;
        }
    }
    return 1;
}

static int parse_int_array(struct ODDLStructure* cur, unsigned int** array, unsigned int* len) {
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

static int parse_float_array(struct ODDLStructure* cur, float** array, unsigned int* len) {
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

static int load_skin_arrays(struct Skin* skin, unsigned int* countArray, unsigned int countLen,
                            unsigned int* indexArray, unsigned int idxLen,
                            float* weightArray, unsigned int weightLen) {
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
        skin->indexArray[2 * cntPos] = topIndices[0];
        skin->indexArray[2 * cntPos + 1] = topIndices[1];
        skin->weightArray[2 * cntPos] = topWeights[0];
        skin->weightArray[2 * cntPos + 1] = topWeights[1];
        idxPos += offset;
    }
    return 1;
}

int ogex_parse_skin(struct OgexContext* context, struct Skin* skin, struct ODDLStructure* cur) {
    unsigned int i, countLen = 0, idxLen = 0, weightLen = 0;
    unsigned int* countArray = NULL;
    unsigned int* indexArray = NULL;
    float* weightArray = NULL;
    int success = 1;

    load_id4(skin->skinTransform);
    for (i = 0; i < cur->nbStructures && success; i++) {
        struct ODDLStructure* tmp = cur->structures[i];

        switch (ogex_get_identifier(tmp)) {
            case OGEX_TRANSFORM:
                if (!ogex_parse_transform(context, tmp, skin->skinTransform)) {
                    success = 0;
                    goto exit;
                }
                break;
            case OGEX_SKELETON:
                if (!parse_skeleton(context, tmp, skin)) {
                    success = 0;
                    goto exit;
                }
                break;
            case OGEX_BONE_COUNT_ARRAY:
                if (!parse_int_array(tmp, &countArray, &countLen)) {
                    success = 0;
                    goto exit;
                }
                break;
            case OGEX_BONE_INDEX_ARRAY:
                if (!parse_int_array(tmp, &indexArray, &idxLen)) {
                    success = 0;
                    goto exit;
                }
                break;
            case OGEX_BONE_WEIGHT_ARRAY:
                if (!parse_float_array(tmp, &weightArray, &weightLen)) {
                    success = 0;
                    goto exit;
                }
                break;
            default:
                break;
        }
    }
    if (!skin->bones || !countArray || !indexArray || !weightArray) {
        fprintf(stderr, "Error: Skin: missing substructure (Skeleton, BoneCountArray, BoneIndexArray, BoneWeightArray)\n");
        success = 0;
        goto exit;
    }
    skin->nbVertices = countLen;
    if (!(skin->indexArray = malloc(2 * countLen * sizeof(*skin->indexArray)))
            || !(skin->weightArray = malloc(2 * countLen * sizeof(*skin->weightArray)))) {
        fprintf(stderr, "Error: Skin: couldn't allocate memory for index or weight array\n");
        success = 0;
        goto exit;
    }
    load_skin_arrays(skin, countArray, countLen, indexArray, idxLen, weightArray, weightLen);
    if (!import_add_shared_item(&context->shared->skins, &context->shared->nbSkins, skin)) {
        success = 0;
        goto exit;
    }
exit:
    free(countArray);
    free(indexArray);
    free(weightArray);
    if (!success) {
        skin_free(skin);
        free(skin);
    }
    return success;
}
