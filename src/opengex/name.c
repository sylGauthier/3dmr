#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "name.h"

static char* strcopy(const char* src) {
    char* ret;
    if ((ret = malloc(strlen(src) + 1))) {
        memcpy(ret, src, strlen(src) + 1);
    }
    return ret;
}

char* ogex_parse_name(const struct ODDLStructure* cur) {
    struct ODDLStructure* oddlString;
    char* str;

    if (cur->nbStructures != 1) {
        fprintf(stderr, "Error: Name: must contain exactly one substructure\n");
        return NULL;
    }
    if (!ogex_check_struct(oddlString = cur->structures[0], "Name", TYPE_STRING, 1, 1)) return NULL;
    str = *((char**)oddlString->dataList);
    return strcopy(str);
}
