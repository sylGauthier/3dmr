#ifndef TEST_SCENES_H
#define TEST_SCENES_H

#include "node.h"
#include "geometry/geometry.h"

void new_geom_nxn(struct Geometry *geom, int len, float space, struct Node *root);

struct Node *box_10x10(struct Geometry *mat, float size, float spacing);

#endif
