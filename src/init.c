#include <stdlib.h>
#include <string.h>
#include <3dmr/math/random.h>

const char* tdmrShaderRootPath;
char* _tdmrShaderRootPath;

int tdmr_init(const char* shaderRootPath) {
    size_t n = strlen(shaderRootPath);
    if (!(_tdmrShaderRootPath = malloc(++n))) {
        return 0;
    }
    memcpy(_tdmrShaderRootPath, shaderRootPath, n);
    tdmrShaderRootPath = _tdmrShaderRootPath;

    random_seed(4357);

    return 1;
}

void tdmr_free(void) {
    free(_tdmrShaderRootPath);
}
