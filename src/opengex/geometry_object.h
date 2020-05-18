#include <liboddl/liboddl.h>
#include <3dmr/scene/node.h>
#include "context.h"

#ifndef TDMR_OPENGEX_GEOMETRY_OBJECT_H
#define TDMR_OPENGEX_GEOMETRY_OBJECT_H

struct Geometry* ogex_parse_geometry_object(struct OgexContext* context, const struct ODDLStructure* cur);
void ogex_free_geometry(void* p);

#endif
