#include <liboddl/liboddl.h>
#include <game/scene/node.h>
#include "context.h"

#ifndef GAME_OPENGEX_GEOMETRY_NODE_H
#define GAME_OPENGEX_GEOMETRY_NODE_H

int ogex_parse_geometry_node(const struct OgexContext* context, struct Node* node, const struct ODDLStructure* cur);

#endif
