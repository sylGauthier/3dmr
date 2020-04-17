#include <stdio.h>
#include <game/scene/scene.h>
#include <game/scene/import.h>

#ifndef GAME_SCENE_OPENGEX_H
#define GAME_SCENE_OPENGEX_H

#ifdef GAME_OPENGEX
int ogex_load(struct Node* root, FILE* ogexFile, const char* path, struct ImportSharedData* shared, struct ImportMetadata* metadata);
#endif

#endif
