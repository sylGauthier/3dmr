#include "context.h"

#ifndef TDMR_OPENGEX_TEXTURE_H
#define TDMR_OPENGEX_TEXTURE_H

int ogex_parse_texture(const struct OgexContext* context, const struct ODDLStructure* cur, char** attrib, GLuint* tex);

#endif
