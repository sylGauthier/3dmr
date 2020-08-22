#ifndef GLTF_LIGHT_NODE_H
#define GLTF_LIGHT_NODE_H

#include "gltf.h"

int gltf_parse_light_node(struct GltfContext* context, struct Node* light, json_t* jnode);

#endif
