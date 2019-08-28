#include <liboddl/liboddl.h>
#include <string.h>

#include "opengex_common.h"

static enum Axis parse_axis(const char* str) {
    if (!strcmp(str, "x")) return AXIS_X;
    if (!strcmp(str, "-x")) return AXIS_MINUS_X;
    if (!strcmp(str, "y")) return AXIS_Y;
    if (!strcmp(str, "-y")) return AXIS_MINUS_Y;
    if (!strcmp(str, "z")) return AXIS_Z;
    if (!strcmp(str, "-z")) return AXIS_MINUS_Z;
    return 0;
}

static int parse_metric(struct OgexContext* context, struct ODDLStructure* cur) {
    struct ODDLProperty* prop;
    struct ODDLStructure* tmp;

    if (!(prop = oddl_get_property(cur, "key"))) {
        fprintf(stderr, "Error: Metric: Metric needs a \"key\" property\n");
        return 0;
    }
    if (cur->nbStructures != 1) {
        fprintf(stderr, "Error: Metric: expected exactly 1 sub structure\n");
        return 0;
    }
    tmp = cur->structures[0];
    if (!strcmp(prop->str, "distance")) {
        if (tmp->type == TYPE_FLOAT32 && tmp->nbVec == 1 && tmp->vecSize == 1) {
            context->scale = *((float*)tmp->dataList);
        } else {
            fprintf(stderr, "Error: Metric: invalid data format\n");
            return 0;
        }
    } else if (!strcmp(prop->str, "angle")) {
        if (tmp->type == TYPE_FLOAT32 && tmp->nbVec == 1 && tmp->vecSize == 1) {
            context->angle = *((float*)tmp->dataList);
        } else {
            fprintf(stderr, "Error: Metric: invalid data format\n");
            return 0;
        }
    } else if (!strcmp(prop->str, "time")) {
        if (tmp->type == TYPE_FLOAT32 && tmp->nbVec == 1 && tmp->vecSize == 1) {
            context->time = *((float*)tmp->dataList);
        } else {
            fprintf(stderr, "Error: Metric: invalid data format\n");
            return 0;
        }
    } else if (!strcmp(prop->str, "up")) {
        if (tmp->type == TYPE_STRING && tmp->nbVec == 1 && tmp->vecSize == 1) {
            char** list = tmp->dataList;
            context->up = parse_axis(list[0]);
            if (!(context->up == AXIS_Y || context->up == AXIS_Z)) {
                fprintf(stderr, "Error: Metric: invalid up axis\n");
                return 0;
            }
        } else {
            fprintf(stderr, "Error: Metric: invalid data format\n");
            return 0;
        }
    } else if (!strcmp(prop->str, "forward")) {
        if (tmp->type == TYPE_STRING && tmp->nbVec == 1 && tmp->vecSize == 1) {
            char** list = tmp->dataList;
            if (!(context->forward = parse_axis(list[0]))) {
                fprintf(stderr, "Error: Metric: invalid forward axis\n");
                return 0;
            }
        } else {
            fprintf(stderr, "Error: Metric: invalid data format\n");
            return 0;
        }
    } else {
        fprintf(stderr, "Error: Metric: unknown key: %s\n", prop->str);
        return 0;
    }
    return 1;
}

static int parse_root(struct OgexContext* context, struct Node* root) {
    unsigned int i;
    struct ODDLStructure* docRoot = context->doc.root;

    for (i = 0; i < docRoot->nbStructures; i++) {
        struct ODDLStructure* cur = docRoot->structures[i];

        switch (ogex_get_identifier(cur)) {
            case OGEX_NONE:
                break;
            case OGEX_CAMERA_OBJECT:
                if (!(ogex_parse_camera_object(context, cur))) return 0;
                break;
            case OGEX_GEOMETRY_OBJECT:
                if (!(ogex_parse_geometry_object(context, cur))) return 0;
                break;
            case OGEX_NODE:
            case OGEX_BONE_NODE:
            case OGEX_CAMERA_NODE:
            case OGEX_GEOMETRY_NODE:
            case OGEX_LIGHT_NODE:
                if (!(ogex_parse_node(context, root, cur))) return 0;
                break;
            case OGEX_MATERIAL:
                if (!(ogex_parse_material(context, cur))) return 0;
                break;
            case OGEX_METRIC:
                if (!(parse_metric(context, cur))) return 0;
                break;
            default:
                break;
        }
    }
    return 1;
}

int ogex_load(struct Node* root, FILE* ogexFile, struct SharedData* shared, struct ImportMetadata* metadata) {
    struct OgexContext context;
    int success;

    context.scale = 1.0;
    context.angle = 1.0;
    context.time = 1.0;
    context.up = AXIS_Z;
    context.forward = AXIS_X;
    context.nbSharedObjects = 0;
    context.sharedObjs = NULL;
    context.root = root;
    context.metadata = metadata;
    context.shared = shared;
    if (shared) {
        memset(shared, 0, sizeof(struct SharedData));
    }
    if (metadata) {
        memset(metadata, 0, sizeof(struct ImportMetadata));
    }
    if (!(oddl_parse(&context.doc, ogexFile))) {
        return 0;
    }
    success = parse_root(&context, root);
#if 0
    if (success) {
        printf("Successfully loaded ogex file.\n");
        printf("    scale = %f\n"
               "    angle = %f\n"
               "    time = %f\n"
               "    up = %d\n"
               "    forward = %d\n", context.scale, context.angle, context.time, context.up, context.forward);
    }
#endif
    oddl_free(&context.doc);
    ogex_free_context(&context);
    return success;
}
