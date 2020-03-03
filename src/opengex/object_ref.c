#include "common.h"
#include "object_ref.h"

const struct ODDLStructure* ogex_parse_object_ref(const struct ODDLStructure* cur) {
    struct ODDLStructure* tmp;
    struct ODDLRef* ref;

    if (cur->nbStructures != 1) {
        fprintf(stderr, "Error: parse_object_ref: invalid nb of structures in ObjectRef\n");
        return NULL;
    }
    if (!ogex_check_struct(tmp = cur->structures[0], "ObjectRef", TYPE_REF, 1, 1)) return NULL;
    ref = tmp->dataList;
    return ref->ref;
}
