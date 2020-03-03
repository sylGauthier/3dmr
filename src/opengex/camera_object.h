#include <game/math/linear_algebra.h>
#include "context.h"

#ifndef GAME_OPENGEX_CAMERA_OBJECT_H
#define GAME_OPENGEX_CAMERA_OBJECT_H

Mat4* ogex_parse_camera_object(const struct OgexContext* context, const struct ODDLStructure* cur);

#endif
