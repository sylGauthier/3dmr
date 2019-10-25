#include <game/material/alpha.h>

void alpha_params_send(GLuint prog, const struct AlphaParams* params, unsigned int* texSlot) {
    if (!params->enabled) return;
    material_param_send_float(prog, &params->alpha, "alpha", texSlot);
}

void alpha_set_defines(enum AlphaParamsFlags flags, const char** defines, unsigned int* numDefines) {
    if (flags & ALPHA_ENABLED) {
        defines[2 * (*numDefines)] = "ALPHA_ENABLED";
        defines[2 * (*numDefines)++ + 1] = NULL;
    }
    if (flags & ALPHA_TEXTURED) {
        defines[2 * (*numDefines)] = "ALPHA_TEXTURED";
        defines[2 * (*numDefines)++ + 1] = NULL;
        defines[2 * (*numDefines)] = "HAVE_TEXCOORD";
        defines[2 * (*numDefines)++ + 1] = NULL;
    }
    if (flags & ALPHA_TEST) {
        defines[2 * (*numDefines)] = "ALPHA_TEST";
        defines[2 * (*numDefines)++ + 1] = NULL;
    }
}
