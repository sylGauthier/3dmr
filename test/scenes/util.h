#include <game/scene/node.h>
#include <game/mesh/mesh.h>

#ifndef TEST_SCENES_UTIL_H
#define TEST_SCENES_UTIL_H

struct Node* create_node(const struct Mesh* mesh, void* material);
void free_node(struct Node* node);

#endif
