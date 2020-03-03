#include <liboddl/liboddl.h>
#include <game/math/linear_algebra.h>

#ifndef GAME_OPENGEX_COLOR_H
#define GAME_OPENGEX_COLOR_H

int ogex_parse_color(const struct ODDLStructure* cur, char** attrib, Vec3 col);

#endif
