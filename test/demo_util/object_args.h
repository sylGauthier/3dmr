#include <game/scene/node.h>

#ifndef DEMO_UTIL_OBJECT_ARGS_H
#define DEMO_UTIL_OBJECT_ARGS_H

void usage_object_args(void);

int parse_object_args(const char* s, char** e, struct Node* object);

#endif
