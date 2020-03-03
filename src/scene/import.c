#include <stdlib.h>
#include <stdio.h>

#include <game/scene/import.h>

void imported_node_free(struct Node* node) {
    if (node) {
        switch (node->type) {
            case NODE_EMPTY:
            case NODE_BONE:
                break;
            case NODE_GEOMETRY:
                if (node->data.geometry) {
                    free(node->data.geometry->material);
                    free(node->data.geometry);
                }
                break;
            case NODE_CAMERA:
                free(node->data.camera);
                break;
            case NODE_DLIGHT:
                free(node->data.dlight);
                break;
            case NODE_PLIGHT:
                free(node->data.plight);
                break;
        }
        free(node);
    }
}

void import_init_shared_data(struct ImportSharedData* shared) {
    shared->matParams = NULL;
    shared->va = NULL;
    shared->skins = NULL;

    shared->numMatParams = 0;
    shared->numVA = 0;
    shared->numSkins = 0;
}

#define IMPORT_ADD_ITEM(type, array, size, newItem) { \
    type** tmp; \
    if ((size) >= ((unsigned int)-1) || !(tmp = realloc((array), ((size) + 1) * sizeof(*(array))))) return 0; \
    (array) = tmp; \
    (array)[(size)++] = newItem; \
    return 1; \
}

int import_add_shared_matparam(struct ImportSharedData* shared, void* p)
IMPORT_ADD_ITEM(void, shared->matParams, shared->numMatParams, p)

int import_add_shared_va(struct ImportSharedData* shared, struct VertexArray* va)
IMPORT_ADD_ITEM(struct VertexArray, shared->va, shared->numVA, va)

int import_add_shared_skin(struct ImportSharedData* shared, struct Skin* s)
IMPORT_ADD_ITEM(struct Skin, shared->skins, shared->numSkins, s)

void import_free_shared_data(struct ImportSharedData* shared) {
    unsigned int i;

    for (i = 0; i < shared->numMatParams; i++) {
        free(shared->matParams[i]);
    }
    for (i = 0; i < shared->numVA; i++) {
        vertex_array_free(shared->va[i]);
    }
    for (i = 0; i < shared->numSkins; i++) {
        skin_free(shared->skins[i]);
    }
    free(shared->matParams);
    free(shared->va);
    free(shared->skins);
}

void import_init_metadata(struct ImportMetadata* metadata) {
    metadata->cameraNodes = NULL;
    metadata->lightNodes = NULL;
    metadata->clips = NULL;

    metadata->numCameraNodes = 0;
    metadata->numLightNodes = 0;
    metadata->numClips = 0;
}

int import_add_metadata_camnode(struct ImportMetadata* metadata, struct Node* n)
IMPORT_ADD_ITEM(struct Node, metadata->cameraNodes, metadata->numCameraNodes, n)

int import_add_metadata_lightnode(struct ImportMetadata* metadata, struct Node* n)
IMPORT_ADD_ITEM(struct Node, metadata->lightNodes, metadata->numLightNodes, n)

int import_add_metadata_clip(struct ImportMetadata* metadata, struct Clip* c)
IMPORT_ADD_ITEM(struct Clip, metadata->clips, metadata->numClips, c)

void import_free_metadata(struct ImportMetadata* metadata) {
    unsigned int i;

    free(metadata->cameraNodes);
    free(metadata->lightNodes);
    for (i = 0; i < metadata->numClips; i++) {
        anim_free_clip(metadata->clips[i]);
    }
    free(metadata->clips);
}
