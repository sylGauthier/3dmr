#include <stdio.h>
#include <3dmr/scene/scene.h>
#include <3dmr/scene/import.h>

#ifndef TDMR_SCENE_GLTF_H
#define TDMR_SCENE_GLTF_H

#ifdef TDMR_GLTF
int gltf_load(struct Node* root, FILE* gltfFile, const char* path, struct ImportSharedData* shared, struct ImportMetadata* metadata, char binary);
#endif

#endif
