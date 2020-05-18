#include <liboddl/liboddl.h>
#include <3dmr/scene/node.h>
#include "context.h"

#ifndef TDMR_OPENGEX_LIGHT_NODE_H
#define TDMR_OPENGEX_LIGHT_NODE_H

int ogex_parse_light_node(const struct OgexContext* context, struct Node* node, const struct ODDLStructure* cur);

#endif
