#ifndef TEST_SCENES_BASIC_H
#define TEST_SCENES_BASIC_H

struct Node *box_10x10(struct Geometry* mat, float size, float spacing);

struct Node *sphere_10x10(struct Geometry* mat, float radius, float spacing);

void spheres_and_boxes(struct Geometry* smat, struct Geometry* bmat, struct Node* root);

#endif
