#include <liboddl/liboddl.h>

#ifndef GAME_OPENGEX_COMMON_H
#define GAME_OPENGEX_COMMON_H

int ogex_check_struct(const struct ODDLStructure* s, const char* parentId, enum ODDLDataType type, unsigned int numVecs, unsigned int vecSize);

#endif
