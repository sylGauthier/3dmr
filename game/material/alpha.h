#include "param.h"

#ifndef GAME_MATERIAL_ALPHA_H
#define GAME_MATERIAL_ALPHA_H

#define ALPHA_MAX_DEFINES 4

struct AlphaParams {
    enum AlphaMode {ALPHA_DISABLED, ALPHA_BLEND, ALPHA_TEST} mode;
    struct MatParamFloat alpha;
};

void alpha_params_init(struct AlphaParams* params);
void alpha_params_send(GLuint prog, const struct AlphaParams* params, unsigned int* texSlot);
void alpha_set_defines(const struct AlphaParams* params, const char** defines, unsigned int* numDefines);

#endif
