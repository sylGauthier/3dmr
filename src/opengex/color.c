#include <string.h>
#include "color.h"
#include "common.h"

int ogex_parse_color(const struct ODDLStructure* cur, char** attrib, Vec3 col) {
    struct ODDLStructure* sub;
    struct ODDLProperty* prop;

    if (!(prop = oddl_get_property(cur, "attrib"))) {
        fprintf(stderr, "Error: Color: property \"attrib\" is required\n");
        return 0;
    }
    if (cur->nbStructures != 1) {
        fprintf(stderr, "Error: Color: invalid number of substructures\n");
        return 0;
    }
    sub = cur->structures[0];
    if (sub->vecSize != 3 && sub->vecSize != 4) {
        fprintf(stderr, "Error: Color: invalid data in substructure, required 1 FLOAT[3/4]\n");
        return 0;
    }
    if (!ogex_check_struct(sub, "Color", TYPE_FLOAT32, 1, sub->vecSize)) return 0;
    *attrib = prop->str;
    memcpy(col, sub->dataList, sizeof(Vec3));
    return 1;
}
