#include <string.h>
#include "axis.h"

enum OgexAxis ogex_parse_axis(const char* str) {
    if (!strcmp(str, "x")) return AXIS_X;
    if (!strcmp(str, "-x")) return AXIS_MINUS_X;
    if (!strcmp(str, "y")) return AXIS_Y;
    if (!strcmp(str, "-y")) return AXIS_MINUS_Y;
    if (!strcmp(str, "z")) return AXIS_Z;
    if (!strcmp(str, "-z")) return AXIS_MINUS_Z;
    return 0;
}
