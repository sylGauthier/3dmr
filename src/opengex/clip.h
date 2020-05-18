#include <liboddl/liboddl.h>
#include "context.h"

#ifndef TDMR_OPENGEX_CLIP_H
#define TDMR_OPENGEX_CLIP_H

struct Clip* ogex_create_new_clip(struct OgexContext* context, unsigned int index);
struct Clip* ogex_parse_clip(struct OgexContext* context, const struct ODDLStructure* cur);

#endif
