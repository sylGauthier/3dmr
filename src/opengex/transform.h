#include "context.h"

#ifndef GAME_OPENGEX_TRANSFORM_H
#define GAME_OPENGEX_TRANSFORM_H

int ogex_parse_translation(const struct OgexContext* context, struct Node* node, const struct ODDLStructure* cur);
int ogex_parse_rotation(const struct OgexContext* context, struct Node* node, const struct ODDLStructure* cur);
int ogex_parse_scale(const struct OgexContext* context, struct Node* node, const struct ODDLStructure* cur);
int ogex_parse_transforms(const struct OgexContext* context, const struct ODDLStructure* cur, Mat4* transforms, unsigned int numTransforms);

int extract_scale(Vec3 scale, Mat4 t);

#endif
