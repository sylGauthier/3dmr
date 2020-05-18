#include <3dmr/scene/node.h>
#include <3dmr/mesh/mesh.h>

#ifndef TEST_SCENES_UTIL_H
#define TEST_SCENES_UTIL_H

struct Node* create_node(const struct Mesh* mesh, struct Material* material, void* fp);
void free_node(struct Node* node);

#endif
