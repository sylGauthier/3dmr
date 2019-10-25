#include "param.h"

#ifndef GAME_MATERIAL_ALPHA_H
#define GAME_MATERIAL_ALPHA_H

#define ALPHA_MAX_DEFINES 4

struct AlphaParams {
    int enabled;
    struct MatParamFloat alpha;
};

enum AlphaParamsFlags {
    ALPHA_ENABLED = 1 << 10,
    ALPHA_TEXTURED = 1 << 11,
    ALPHA_TEST = 1 << 12
};

void alpha_params_send(GLuint prog, const struct AlphaParams* params, unsigned int* texSlot);
void alpha_set_defines(enum AlphaParamsFlags flags, const char** defines, unsigned int* numDefines);

#endif
