#include "gltf.h"

void gltf_context_free(struct GltfContext* context) {
    unsigned int i;

    free(context->nodes);
    free(context->skins);
    free(context->lights);
    free(context->cameras);
    free(context->meshes);
    free(context->materials);
    for (i = 0; i < context->numBuffers; i++) free(context->buffers[i].data);
    free(context->buffers);
    free(context->bufferViews);
    free(context->accessors);
    free(context->textures);
    free(context->clips);
}

static void restore_user_struct(struct GltfContext* context) {
    context->shared->numMatParams = context->sharedSave.numMatParams;
    context->shared->numVA = context->sharedSave.numVA;
    context->shared->numSkins = context->sharedSave.numSkins;
    context->shared->numGeometries = context->sharedSave.numGeometries;

    context->metadata->numCameraNodes = context->metadataSave.numCameraNodes;
    context->metadata->numLightNodes = context->metadataSave.numLightNodes;
    context->metadata->numClips = context->metadataSave.numClips;

    context->root->nbChildren = context->nbChildrenSave;
}

void gltf_context_drop(struct GltfContext* context) {
    unsigned int i;

    for (i = 0; i < context->numNodes; i++) {
        if (context->nodes[i]) {
            free(context->nodes[i]->children);
            free(context->nodes[i]->name);
            free(context->nodes[i]);
        }
    }
    for (i = 0; i < context->numSkins; i++) {
        skin_free(context->skins[i]);
    }
    for (i = 0; i < context->numMeshes; i++) {
        if (context->meshes[i]) {
            vertex_array_free(context->meshes[i]->vertexArray);
            free(context->meshes[i]->material);
            free(context->meshes[i]);
        }
    }
    for (i = 0; i < context->numCameras; i++) free(context->cameras[i]);
    for (i = 0; i < context->numMaterials; i++) free(context->materials[i]);
    for (i = 0; i < context->numClips; i++) {
        anim_clip_free(context->clips[i]);
        free(context->clips[i]);
    }
    gltf_context_free(context);
    restore_user_struct(context);
}
