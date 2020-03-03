#include <liboddl/liboddl.h>
#include <game/scene/skin.h>
#include "context.h"

#ifndef GAME_OPENGEX_SKIN_H
#define GAME_OPENGEX_SKIN_H

int ogex_post_parse_skeletons(struct OgexContext* context);
int ogex_parse_skin(struct OgexContext* context, struct ODDLStructure* cur, struct Skin** skin, unsigned int** indices, float** weights, unsigned int* numVertices);

#endif
