#include <string.h>

#include "gltf.h"
#include "utils.h"

struct GltfAccessor* gltf_get_acc(struct GltfContext* context, unsigned int idx) {
    if (idx >= context->numAccessors) {
        fprintf(stderr, "Error: gltf: invalid accessor index\n");
        return NULL;
    }
    return &context->accessors[idx];
}

void* gltf_acc_get_buf(struct GltfContext* context, struct GltfAccessor* acc, unsigned int* byteStride) {
    struct GltfBufferView* view;
    struct GltfBuffer* buf;

    view = &context->bufferViews[acc->bufferView];
    buf = &context->buffers[view->buffer];
    if (byteStride) *byteStride = view->byteStride;
    return ((char*)buf->data) + acc->byteOffset + view->byteOffset;
}

int gltf_parse_buffers(struct GltfContext* context, json_t* jroot) {
    unsigned int idx;
    json_t *buf, *buffers;

    if (       !(buffers = json_object_get(jroot, "buffers"))
            || !(context->numBuffers = json_array_size(buffers))) {
        return 1;
    }
    if (!(context->buffers = malloc(context->numBuffers * sizeof(*context->buffers)))) {
        fprintf(stderr, "Error: gltf: can't allocate memory for buffers\n");
        return 0;
    }
    json_array_foreach(buffers, idx, buf) {
        json_t* uri;

        if (!(context->buffers[idx].size = json_integer_value(json_object_get(buf, "byteLength")))) {
            fprintf(stderr, "Error: gltf: invalid buffer size\n");
            return 0;
        }
        if (!(uri = json_object_get(buf, "uri"))) {
            if (idx != 0 || !context->binary) {
                fprintf(stderr, "Error: gltf: missing uri (either not a binary file or not first buffer in the list)\n");
                return 0;
            }
        }
        if (!(context->buffers[idx].data = malloc(context->buffers[idx].size))) {
            fprintf(stderr, "Error: gltf: could not allocate memory for buffer\n");
            return 0;
        }
        if (uri) {
            if (!gltf_load_uri(context, uri, &context->buffers[idx])) {
                fprintf(stderr, "Error: gltf: could not load uri\n");
                return 0;
            }
        } else {
            if (fread(context->buffers[idx].data, 1, context->buffers[idx].size, context->file) != context->buffers[idx].size) {
                fprintf(stderr, "Error: gltf: buffer: error reading file's own binary data\n");
                return 0;
            }
        }
    }
    return 1;
}

int gltf_parse_buffer_views(struct GltfContext* context, json_t* jroot) {
    unsigned int idx;
    json_t *curView, *views;

    if (       !(views = json_object_get(jroot, "bufferViews"))
            || !(context->numBufferViews = json_array_size(views))) {
        return 1;
    }
    if (!(context->bufferViews = malloc(context->numBufferViews * sizeof(*context->bufferViews)))) {
        fprintf(stderr, "Error: gltf: can't allocate memory for buffer views\n");
        return 0;
    }
    json_array_foreach(views, idx, curView) {
        context->bufferViews[idx].buffer = json_integer_value(json_object_get(curView, "buffer"));
        context->bufferViews[idx].byteLength = json_integer_value(json_object_get(curView, "byteLength"));
        context->bufferViews[idx].byteOffset = json_integer_value(json_object_get(curView, "byteOffset"));
        context->bufferViews[idx].byteStride = json_integer_value(json_object_get(curView, "byteStride"));

        if (context->bufferViews[idx].buffer >= context->numBuffers) {
            fprintf(stderr, "Error: gltf: bufferView: invalid buffer index\n");
            return 0;
        }
        if (!context->bufferViews[idx].byteLength) {
            fprintf(stderr, "Error: gltf: bufferView: byteLength is zero\n");
        }
        if (context->bufferViews[idx].byteLength + context->bufferViews[idx].byteOffset
                > context->buffers[context->bufferViews[idx].buffer].size) {
            fprintf(stderr, "Error: gltf: bufferView: invalid length and offset\n");
            return 0;
        }
    }
    return 1;
}

static int check_mem_bounds(struct GltfContext* context, struct GltfAccessor* acc) {
    struct GltfBufferView* v;
    unsigned int stride;

    v = context->bufferViews + acc->bufferView;
    if (v->byteStride) {
        stride = v->byteStride;
    } else {
        stride = GLTF_TYPE_SIZE(acc->componentType) * GLTF_COMP_SIZE(acc->type);
    }
    return acc->count * stride <= v->byteLength;
}

int gltf_parse_accessors(struct GltfContext* context, json_t* jroot) {
    unsigned int idx;
    json_t *curAccessor, *accessors;

    if (       !(accessors = json_object_get(jroot, "accessors"))
            || !(context->numAccessors = json_array_size(accessors))) {
        return 1;
    }
    if (!(context->accessors = malloc(context->numAccessors * sizeof(*context->accessors)))) {
        fprintf(stderr, "Error: gltf: can't allocate memory for buffer views\n");
        return 0;
    }
    json_array_foreach(accessors, idx, curAccessor) {
        const char* type;

        context->accessors[idx].bufferView = json_integer_value(json_object_get(curAccessor, "bufferView"));
        context->accessors[idx].byteOffset = json_integer_value(json_object_get(curAccessor, "byteOffset"));
        context->accessors[idx].componentType = json_integer_value(json_object_get(curAccessor, "componentType"));
        context->accessors[idx].count = json_integer_value(json_object_get(curAccessor, "count"));
        type = json_string_value(json_object_get(curAccessor, "type"));

        if (!type) {
            fprintf(stderr, "Error: gltf: accessor: missing type\n");
            return 0;
        }
        if (!strcmp(type, "SCALAR")) {
            context->accessors[idx].type = GLTF_SCALAR;
        } else if (!strcmp(type, "VEC2")) {
            context->accessors[idx].type = GLTF_VEC2;
        } else if (!strcmp(type, "VEC3")) {
            context->accessors[idx].type = GLTF_VEC3;
        } else if (!strcmp(type, "VEC4")) {
            context->accessors[idx].type = GLTF_VEC4;
        } else if (!strcmp(type, "MAT2")) {
            context->accessors[idx].type = GLTF_MAT2;
        } else if (!strcmp(type, "MAT3")) {
            context->accessors[idx].type = GLTF_MAT3;
        } else if (!strcmp(type, "MAT4")) {
            context->accessors[idx].type = GLTF_MAT4;
        } else {
            fprintf(stderr, "Error: gltf: accessor: invalid type: %s\n", type);
            return 0;
        }
        if (context->accessors[idx].bufferView >= context->numBufferViews) {
            fprintf(stderr, "Error: gltf: accessor: invalid bufferView index\n");
            return 0;
        }
        if (context->accessors[idx].componentType != GLTF_BYTE
                && context->accessors[idx].componentType != GLTF_UNSIGNED_BYTE
                && context->accessors[idx].componentType != GLTF_SHORT
                && context->accessors[idx].componentType != GLTF_UNSIGNED_SHORT
                && context->accessors[idx].componentType != GLTF_UNSIGNED_INT
                && context->accessors[idx].componentType != GLTF_FLOAT) {
            fprintf(stderr, "Error: gltf: invalid componentType: %d\n", context->accessors[idx].componentType);
            return 0;
        }
        if (!check_mem_bounds(context, &context->accessors[idx])) {
            fprintf(stderr, "Error: gltf: accessor: invalid parameters (mem bounds)\n");
            return 0;
        }
    }
    return 1;
}
