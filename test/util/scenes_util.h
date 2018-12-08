#include <game/render/globject.h>
#include <game/scene/node.h>

#ifndef TEST_SCENES_UTIL_H
#define TEST_SCENES_UTIL_H

void new_geom_surface(struct GLObject* o, int slen, float spacing, struct Node* root);

void translate_to_center(const int slen, const float spacing, struct Node* node);

#endif
