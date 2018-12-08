#include <game/render/globject.h>
#include <game/render/material.h>
#include <game/scene/node.h>

#ifndef TEST_SCENES_BASIC_H
#define TEST_SCENES_BASIC_H

struct Node* box_surface(struct Material* mat, float size, int slen, float spacing);

struct Node* sphere_surface(struct Material* mat, float radius, int slen, float spacing);

void spheres_and_boxes(struct Material* smat, struct Material* bmat, struct Node* root);

#endif
