#include <3dmr/math/linear_algebra.h>
#include "context.h"

#ifndef TDMR_OPENGEX_CAMERA_OBJECT_H
#define TDMR_OPENGEX_CAMERA_OBJECT_H

Mat4* ogex_parse_camera_object(const struct OgexContext* context, const struct ODDLStructure* cur);

#endif
