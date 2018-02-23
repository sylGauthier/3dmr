#ifndef TEST_SCENES_UTIL_H
#define TEST_SCENES_UTIL_H

#include "node.h"
#include "geometry/geometry.h"

void new_geom_surface(struct Geometry* geom, int slen, float spacing, struct Node* root);

void translate_to_center(const int slen, const float spacing, struct Node* node);

#endif
