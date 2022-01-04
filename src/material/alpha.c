#include <3dmr/material/alpha.h>
#include <3dmr/render/shader.h>

void alpha_params_init(struct AlphaParams* params) {
    params->mode = ALPHA_DISABLED;
    material_param_set_float_constant(&params->alpha, 1);
}

void alpha_params_send(GLuint prog, const struct AlphaParams* params) {
    if (!params->mode) return;
    material_param_send_float(prog, &params->alpha, "alpha", TEX_SLOT_COLOR_ALPHA);
}

void alpha_set_defines(const struct AlphaParams* params, const char** defines, unsigned int* numDefines) {
    if (!params->mode) return;
    defines[2 * (*numDefines)] = "ALPHA_ENABLED";
    defines[2 * (*numDefines)++ + 1] = NULL;
    if (params->alpha.mode == MAT_PARAM_TEXTURED) {
        defines[2 * (*numDefines)] = "ALPHA_TEXTURED";
        defines[2 * (*numDefines)++ + 1] = NULL;
    }
    if (params->mode == ALPHA_TEST) {
        defines[2 * (*numDefines)] = "ALPHA_TEST";
        defines[2 * (*numDefines)++ + 1] = NULL;
    }
}
