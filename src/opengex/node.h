#include <liboddl/liboddl.h>
#include "context.h"

#ifndef TDMR_OPENGEX_NODE_H
#define TDMR_OPENGEX_NODE_H

int ogex_parse_nodes(struct OgexContext* context, const struct ODDLStructure* docRoot, struct Node* root);

#endif
