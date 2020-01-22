#include <stdlib.h>
#include <string.h>

#include "opengex_common.h"

static int parse_bone_ref_array(struct OgexContext* context, struct ODDLStructure* cur, struct Node** bones, unsigned int numBones) {
    struct ODDLStructure* tmp;
    struct ODDLRef* refs;
    unsigned int i;

    if (cur->nbStructures != 1) {
        fprintf(stderr, "Error: BoneRefArray: must contain exactly one substructure\n");
        return 0;
    }
    tmp = cur->structures[0];
    if (tmp->type != TYPE_REF || tmp->vecSize != 1) {
        fprintf(stderr, "Error: BoneRefArray: substructure must be an array of single refs\n");
        return 0;
    }
    if (tmp->nbVec != numBones) {
        fprintf(stderr, "Error: BoneRefArray: inconsistent number of bones\n");
        return 0;
    }
    refs = tmp->dataList;
    for (i = 0; i < tmp->nbVec; i++) {
        struct ODDLStructure* ref = refs[i].ref;
        if (!(ogex_get_identifier(ref) == OGEX_BONE_NODE)) {
            fprintf(stderr, "Error: BoneRefArray: a ref points to an object that isn't a bone\n");
            return 0;
        }
        if (!(bones[i] = ogex_get_shared_object(context, ref))) {
            fprintf(stderr, "Error: BoneRefArray: a ref points to a bone node that doesn't exist, make sure the skeleton is declared after all bone nodes\n");
            return 0;
        }
    }
    return 1;
}

static int parse_bind_pose(struct OgexContext* context, struct ODDLStructure* cur, Mat4* transforms, unsigned int numTransforms) {
    return ogex_parse_transforms(context, cur, transforms, numTransforms);
}

static int parse_skeleton(struct OgexContext* context, struct ODDLStructure* cur, struct Skin** skin) {
    unsigned int i, *n, numBones = 0, numTransforms = 0;

    for (i = 0; i < cur->nbStructures; i++) {
        struct ODDLStructure* tmp = cur->structures[i];

        switch (ogex_get_identifier(tmp)) {
            case OGEX_BONE_REF_ARRAY: n = &numBones; break;
            case OGEX_TRANSFORM: n = &numTransforms; break;
            default: continue;
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
    if (numBones != numTransforms) {;
        fprintf(stderr, "Error: Skeleton: inconsistent number of bind pose transforms\n");
        return 0;
    }
    if (!(*skin = skin_new(numBones))) {
        fprintf(stderr, "Error: Mesh: could not allocate memory for Skin\n");
        return 0;
    }
    if (!import_add_shared_item(&context->shared->skins, &context->shared->nbSkins, *skin)) {
        skin_free(*skin);
        return 0;
    }
    for (i = 0; i < cur->nbStructures; i++) {
        struct ODDLStructure* tmp = cur->structures[i];

        switch (ogex_get_identifier(tmp)) {
            case OGEX_BONE_REF_ARRAY:
                if (!parse_bone_ref_array(context, tmp, (*skin)->bones, numBones)) return 0;
                break;
            case OGEX_TRANSFORM:
                if (!parse_bind_pose(context, tmp, (*skin)->bindPose, numBones)) return 0;
                break;
            default:;
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

int ogex_parse_skin(struct OgexContext* context, struct ODDLStructure* cur, struct Skin** skin, unsigned int** indices, float** weights, unsigned int* numVertices) {
    Mat4 skinTransform;
    unsigned int i, countLen = 0, idxLen = 0, weightLen = 0;
    unsigned int* countArray = NULL;
    unsigned int* indexArray = NULL;
    float* weightArray = NULL;
    int success = 1;

    *skin = NULL;
    load_id4(skinTransform);
    for (i = 0; i < cur->nbStructures && success; i++) {
        struct ODDLStructure* tmp = cur->structures[i];

        switch (ogex_get_identifier(tmp)) {
            case OGEX_TRANSFORM:
                success = ogex_parse_transforms(context, tmp, &skinTransform, 1);
                break;
            case OGEX_SKELETON:
                success = !*skin && parse_skeleton(context, tmp, skin);
                break;
            case OGEX_BONE_COUNT_ARRAY:
                success = !countArray && parse_int_array(tmp, &countArray, &countLen);
                break;
            case OGEX_BONE_INDEX_ARRAY:
                success = !indexArray && parse_int_array(tmp, &indexArray, &idxLen);
                break;
            case OGEX_BONE_WEIGHT_ARRAY:
                success = !weightArray && parse_float_array(tmp, &weightArray, &weightLen);
                break;
            default:;
        }
    }
    if (success) {
        success = 0;
        if (!*skin || !(*skin)->bones || !countArray || !indexArray || !weightArray) {
            fprintf(stderr, "Error: Skin: missing substructure (Skeleton, BoneCountArray, BoneIndexArray, BoneWeightArray)\n");
        } else {
            *indices = malloc(2 * countLen * sizeof(*indices));
            *weights = malloc(2 * countLen * sizeof(*weights));
            *numVertices = countLen;
            if (!*indices || !*weights) {
                fprintf(stderr, "Error: Skin: couldn't allocate memory for index or weight array\n");
            } else {
                load_skin_arrays(countArray, countLen, indexArray, idxLen, weightArray, weightLen, *indices, *weights);
                memcpy((*skin)->skinTransform, skinTransform, sizeof(Mat4));
                success = 1;
            }
            if (!success) {
                free(*indices);
                free(*weights);
            }
        }
    }
    free(countArray);
    free(indexArray);
    free(weightArray);
    return success;
}
