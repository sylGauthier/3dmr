#include "common.h"

int ogex_check_struct(const struct ODDLStructure* s, const char* parentId, enum ODDLDataType type, unsigned int numVecs, unsigned int vecSize) {
    if (s->type != type) {
        fprintf(stderr, "Error: in %s: invalid substructure type, required %s\n", parentId, typeName[type]);
        return 0;
    }
    if (s->nbVec != numVecs || s->vecSize != vecSize) {
        fprintf(stderr, "Error: in %s: invalid substructure size, expected %u vectors of size %u but got %u vectors of size %u\n", parentId, numVecs, vecSize, s->nbVec, s->vecSize);
        return 0;
    }
    return 1;
}
