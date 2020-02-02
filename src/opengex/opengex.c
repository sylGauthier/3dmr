#include <stdlib.h>
#include <string.h>
#include <liboddl/liboddl.h>

#include "clip.h"
#include "context.h"
#include "camera_object.h"
#include "geometry_object.h"
#include "light_object.h"
#include "material.h"
#include "metric.h"
#include "node.h"
#include "skin.h"

static int ogex_parse_toplevels(struct OgexContext* context, struct ODDLStructure* root) {
    unsigned int i;

    for (i = 0; i < root->nbStructures; i++) {
        struct ODDLStructure* cur = root->structures[i];

        if (!cur->identifier) {
            continue;
        } else if (!strcmp(cur->identifier, "CameraObject")) {
            Mat4* c;
            if (!(c = ogex_parse_camera_object(context, cur))) return 0;
            if (!ogex_context_add_object(&context->cameras, cur, c)) {
                free(c);
                return 0;
            }
        } else if (!strcmp(cur->identifier, "Clip")) {
            if (!(ogex_parse_clip(context, cur))) return 0;
        } else if (!strcmp(cur->identifier, "GeometryObject")) {
            struct Geometry* g;
            if (!(g = ogex_parse_geometry_object(context, cur))) return 0;
            if (!ogex_context_add_object(&context->geometries, cur, g)) {
                ogex_free_geometry(g);
                return 0;
            }
        } else if (!strcmp(cur->identifier, "LightObject")) {
            struct OgexLight* l;
            if (!(l = ogex_parse_light_object(context, cur))) return 0;
            if (!ogex_context_add_object(&context->lights, cur, l)) {
                free(l);
                return 0;
            }
        } else if (!strcmp(cur->identifier, "Material")) {
            struct PhongMaterialParams* params;
            if (!(params = ogex_parse_material(context, cur))) return 0;
            if (!ogex_context_add_object(&context->matParams, cur, params)) {
                ogex_free_material(params);
                return 0;
            }
        }
    }
    return 1;
}

static int ogex_parse_all(struct ODDLDoc* doc, struct OgexContext* context, struct Node* root) {
    return ogex_parse_metrics(context, doc->root)
        && ogex_parse_toplevels(context, doc->root)
        && ogex_parse_nodes(context, doc->root, root)
        && ogex_post_parse_skeletons(context);
}

int ogex_load(struct Node* root, FILE* ogexFile, const char* path, struct ImportSharedData* shared, struct ImportMetadata* metadata) {
    struct OgexContext context;
    struct ODDLDoc doc;
    unsigned int i, numRootChildren = root->nbChildren;
    int success;

    ogex_init_context(&context);
    context.path = path;
    context.metadata = metadata;
    if (!(oddl_parse(&doc, ogexFile))) {
        return 0;
    }
    success = ogex_parse_all(&doc, &context, root);
    if (success && shared) {
        struct VertexArray** newVA = NULL;
        void** newMP = NULL;
        struct Skin** newSk = NULL;
        if (context.geometries.num < ((unsigned int)-1) - shared->numVA) {
            if ((newVA = realloc(shared->va, (shared->numVA + context.geometries.num) * sizeof(*shared->va)))) shared->va = newVA;
        }
        if (context.matParams.num < ((unsigned int)-1) - shared->numMatParams) {
            if ((newMP = realloc(shared->matParams, (shared->numMatParams + context.matParams.num) * sizeof(*shared->matParams)))) shared->matParams = newMP;
        }
        if (context.skins.num < ((unsigned int)-1) - shared->numSkins) {
            if ((newSk = realloc(shared->skins, (shared->numSkins + context.skins.num) * sizeof(*shared->skins)))) shared->skins = newSk;
        }
        if (!newVA || !newMP || !newSk) {
            fprintf(stderr, "Error: failed to allocate imported shared data arrays\n");
            success = 0;
        }
    }
    if (success && metadata) {
        struct Clip** newClips = NULL;
        if (context.numClips < ((unsigned int)-1) - metadata->numClips) {
            if ((newClips = realloc(metadata->clips, (metadata->numClips + context.numClips) * sizeof(*metadata->clips)))) metadata->clips = newClips;
        }
    }
    if (success) {
        for (i = 0; i < context.geometries.num; i++) {
            struct Geometry* g = context.geometries.objects[i];
            if (shared) shared->va[shared->numVA++] = g->vertexArray;
            g->vertexArray = NULL;
        }
        for (i = 0; i < context.matParams.num; i++) {
            if (shared) shared->matParams[shared->numMatParams++] = context.matParams.objects[i];
            context.matParams.objects[i] = NULL;
        }
        context.matParams.num = 0;
        for (i = 0; i < context.skins.num; i++) {
            if (shared) shared->skins[shared->numSkins++] = context.skins.objects[i];
            context.skins.objects[i] = NULL;
        }
        context.skins.num = 0;
        if (metadata) {
            for (i = 0; i < context.numClips; i++) {
                if (context.clips[i]) {
                    metadata->clips[metadata->numClips++] = context.clips[i];
                    context.clips[i] = NULL;
                }
            }
        }
    } else {
        if (metadata) import_free_metadata(metadata);
        while (root->nbChildren > numRootChildren) {
            nodes_free(root->children[--(root->nbChildren)], imported_node_free);
        }
    }
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
    oddl_free(&doc);
    ogex_free_context(&context);
    return success;
}
