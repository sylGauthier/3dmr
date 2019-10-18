#include <stdio.h>
#include <game/scene/scene.h>
#include <game/scene/import.h>

#ifndef OPENGEX_H
#define OPENGEX_H

#ifdef GAME_OPENGEX
int ogex_load(struct Node* root, FILE* ogexFile, const char* path, struct SharedData* shared, struct ImportMetadata* metadata);
#endif

#endif
