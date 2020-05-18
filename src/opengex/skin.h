#include <liboddl/liboddl.h>
#include <3dmr/scene/skin.h>
#include "context.h"

#ifndef TDMR_OPENGEX_SKIN_H
#define TDMR_OPENGEX_SKIN_H

int ogex_post_parse_skeletons(struct OgexContext* context);
int ogex_parse_skin(struct OgexContext* context, struct ODDLStructure* cur, struct Skin** skin, unsigned int** indices, float** weights, unsigned int* numVertices);

#endif
