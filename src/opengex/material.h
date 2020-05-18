#include <3dmr/material/phong.h>
#include "context.h"

#ifndef TDMR_OPENGEX_MATERIAL_H
#define TDMR_OPENGEX_MATERIAL_H

struct PhongMaterialParams* ogex_parse_material(const struct OgexContext* context, const struct ODDLStructure* cur);
void ogex_free_material(void* p);

#endif
