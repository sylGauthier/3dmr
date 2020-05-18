#include <liboddl/liboddl.h>

#ifndef TDMR_OPENGEX_COMMON_H
#define TDMR_OPENGEX_COMMON_H

int ogex_check_struct(const struct ODDLStructure* s, const char* parentId, enum ODDLDataType type, unsigned int numVecs, unsigned int vecSize);

#endif
