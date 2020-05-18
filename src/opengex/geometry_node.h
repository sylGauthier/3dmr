#include <liboddl/liboddl.h>
#include <3dmr/scene/node.h>
#include "context.h"

#ifndef TDMR_OPENGEX_GEOMETRY_NODE_H
#define TDMR_OPENGEX_GEOMETRY_NODE_H

int ogex_parse_geometry_node(const struct OgexContext* context, struct Node* node, const struct ODDLStructure* cur);

#endif
