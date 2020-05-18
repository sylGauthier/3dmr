#include <liboddl/liboddl.h>
#include <3dmr/math/linear_algebra.h>

#ifndef TDMR_OPENGEX_COLOR_H
#define TDMR_OPENGEX_COLOR_H

int ogex_parse_color(const struct ODDLStructure* cur, char** attrib, Vec3 col);

#endif
