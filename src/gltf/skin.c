#include <string.h>

#include "gltf.h"

static int read_mat4_array(struct GltfContext* context, struct GltfAccessor* acc, Mat4* array) {
    struct GltfBufferView* view;
    struct GltfBuffer* buffer;
    char* data;
    unsigned int i;

    if (acc->type != GLTF_MAT4) {
        fprintf(stderr, "Error: gltf: skin: bindPose should have type MAT4\n");
        return 0;
    }
    if (!(acc->componentType == GLTF_FLOAT)) {
        fprintf(stderr, "Error: gltf: skin: bindPose component type should be FLOAT\n");
        return 0;
    }
    view = &context->bufferViews[acc->bufferView];
    buffer = &context->buffers[view->buffer];
    data = ((char*) buffer->data) + acc->byteOffset + view->byteOffset;
    for (i = 0; i < acc->count; i++) {
        Mat4 tmp;
        memcpy(tmp, data + i * sizeof(Mat4), sizeof(Mat4));
        invert4m(array[i], MAT_CONST_CAST(tmp));
    }
    return 1;
}

static int apply_skins(struct GltfContext* context, json_t* jroot) {
    json_t *nodes, *curNode;
    unsigned int idx;

    if (!(nodes = json_object_get(jroot, "nodes"))) return 1;

    json_array_foreach(nodes, idx, curNode) {
        json_t *jskin;

        if ((jskin = json_object_get(curNode, "skin")) && json_is_integer(jskin)) {
            unsigned int skinIdx;
            struct Skin* skin;
            struct Node* node;

            skinIdx = json_integer_value(jskin);
            if (skinIdx >= context->numSkins) {
                fprintf(stderr, "Error: gltf: node: invalid skin index\n");
                return 0;
            }
            skin = context->skins[skinIdx];
            node = context->nodes[idx];
            if (node->type != NODE_GEOMETRY) {
                fprintf(stderr, "Warning: gltf: node: trying to skin a non geometry node\n");
            } else {
                vertex_array_set_skin(node->data.geometry->vertexArray, skin);
            }
        }
    }
    return 1;
}

int gltf_parse_skins(struct GltfContext* context, json_t* jroot) {
    json_t *tmp, *cur;
    unsigned int idx;

    if (       !(tmp = json_object_get(jroot, "skins"))
            || !json_array_size(tmp)) {
        return 1;
    }
    if (!(context->skins = calloc(json_array_size(tmp), sizeof(*context->skins)))) {
        fprintf(stderr, "Error: gltf: skin: can't allocate memory for skin array\n");
        return 0;
    }
    context->numSkins = json_array_size(tmp);
    json_array_foreach(tmp, idx, cur) {
        json_t* tmp2;
        struct GltfAccessor* acc = NULL;
        unsigned int i, numBones;

        if (       !(tmp2 = json_object_get(cur, "inverseBindMatrices"))
                || !json_is_integer(tmp2)) {
            fprintf(stderr, "Error: gltf: skin: missing inverseBindMatrices\n");
            return 0;
        }
        if (json_integer_value(tmp2) >= context->numAccessors) {
            fprintf(stderr, "Error: gltf: skin: inverseBindMatrices: invalid accessor ID\n");
            return 0;
        }
        acc = &context->accessors[json_integer_value(tmp2)];
        if (       !(tmp2 = json_object_get(cur, "joints"))
                || !json_is_array(tmp2)) {
            fprintf(stderr, "Error: gltf: skin: missing joints\n");
            return 0;
        }
        numBones = json_array_size(tmp2);
        if (!(context->skins[idx] = skin_new(numBones))) {
            fprintf(stderr, "Error: gltf: skin: can't generate Skin\n");
            return 0;
        }
        for (i = 0; i < numBones; i++) {
            unsigned int bone;
            bone = json_integer_value(json_array_get(tmp2, i));
            if (bone >= context->numNodes) {
                fprintf(stderr, "Error: gltf: skin: invalid bone index\n");
                return 0;
            }
            context->skins[idx]->bones[i] = context->nodes[bone];
        }
        if (numBones != acc->count) {
            fprintf(stderr, "Error: gltf: skin: different number of bones and inverseBindMatrices\n");
            return 0;
        }
        if (!read_mat4_array(context, acc, context->skins[idx]->bindPose)) return 0;
        if (context->shared) {
            if (!(import_add_shared_skin(context->shared, context->skins[idx]))) return 0;
        }
    }
    return apply_skins(context, jroot);
}
