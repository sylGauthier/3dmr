#include <liboddl/liboddl.h>
#include <game/scene/node.h>
#include "context.h"

#ifndef GAME_OPENGEX_LIGHT_NODE_H
#define GAME_OPENGEX_LIGHT_NODE_H

int ogex_parse_light_node(const struct OgexContext* context, struct Node* node, const struct ODDLStructure* cur);

#endif
