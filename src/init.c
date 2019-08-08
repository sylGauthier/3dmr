#include <stdlib.h>
#include <string.h>
#include <game/math/random.h>
#include "material/programs.h"

int game_init(const char* gameShaderRootPath) {
    size_t n = strlen(gameShaderRootPath);
    if (!(shaderRootPath = malloc(++n))) {
        return 0;
    }
    memcpy(shaderRootPath, gameShaderRootPath, n);

    random_seed(4357);

    return 1;
}

void game_free(void) {
    free(shaderRootPath);
}
