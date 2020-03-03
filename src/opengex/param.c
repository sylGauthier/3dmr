#include "common.h"
#include "param.h"

int ogex_parse_param(const struct ODDLStructure* cur, char** attrib, float* value) {
    struct ODDLProperty* prop;

    if (!(prop = oddl_get_property(cur, "attrib"))) {
        fprintf(stderr, "Error: Param: missing property \"attrib\"\n");
        return 0;
    }
    if (cur->nbStructures != 1) {
        fprintf(stderr, "Error: Param: invalid number of substructures\n");
        return 0;
    }
    if (!ogex_check_struct(cur->structures[0], "Param", TYPE_FLOAT32, 1, 1)) return 0;
    *attrib = prop->str;
    *value = *((float*)(cur->structures[0]->dataList));
    return 1;
}
