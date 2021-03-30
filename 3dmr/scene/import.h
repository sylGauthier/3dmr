#include <3dmr/animation/animation.h>
#include <3dmr/scene/node.h>
#include <3dmr/scene/skin.h>

#ifndef TDMR_SCENE_IMPORT_H
#define TDMR_SCENE_IMPORT_H

void imported_node_free(struct Node* node);

struct ImportSharedData {
    void** matParams;
    struct VertexArray** va;
    struct Skin** skins;
    struct Geometry** geometries;

    unsigned int numMatParams;
    unsigned int numVA;
    unsigned int numSkins;
    unsigned int numGeometries;
};

void import_init_shared_data(struct ImportSharedData* shared);
void import_free_shared_data(struct ImportSharedData* shared);

int import_add_shared_matparam(struct ImportSharedData* shared, void* p);
int import_add_shared_va(struct ImportSharedData* shared, struct VertexArray* va);
int import_add_shared_skin(struct ImportSharedData* shared, struct Skin* s);
int import_add_shared_geometry(struct ImportSharedData* shared, struct Geometry* g);

struct ImportMetadata {
    struct Node** cameraNodes;
    struct Node** lightNodes;
    struct Clip** clips;

    unsigned int numCameraNodes;
    unsigned int numLightNodes;
    unsigned int numClips;
};


void import_init_metadata(struct ImportMetadata* metadata);
void import_free_metadata(struct ImportMetadata* metadata);

int import_add_metadata_camnode(struct ImportMetadata* metadata, struct Node* n);
int import_add_metadata_lightnode(struct ImportMetadata* metadata, struct Node* n);
int import_add_metadata_clip(struct ImportMetadata* metadata, struct Clip* c);

struct ImportOptions {
    struct IBL* ibl;
    char binary;
};

void import_init_options(struct ImportOptions* opts);

#endif
