#include <liboddl/liboddl.h>
#include "context.h"

#ifndef GAME_OPENGEX_CLIP_H
#define GAME_OPENGEX_CLIP_H

char* ogex_parse_name(const struct ODDLStructure* cur);
struct Clip* ogex_create_new_clip(struct OgexContext* context, unsigned int index);
struct Clip* ogex_parse_clip(struct OgexContext* context, const struct ODDLStructure* cur);

#endif
