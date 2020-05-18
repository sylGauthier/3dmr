#include <stdio.h>
#include <3dmr/scene/scene.h>
#include <3dmr/scene/import.h>

#ifndef TDMR_SCENE_OPENGEX_H
#define TDMR_SCENE_OPENGEX_H

#ifdef TDMR_OPENGEX
int ogex_load(struct Node* root, FILE* ogexFile, const char* path, struct ImportSharedData* shared, struct ImportMetadata* metadata);
#endif

#endif
