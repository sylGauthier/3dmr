#include <stdlib.h>
#include <string.h>
#include <game/math/random.h>

const char* shaderRootPath;
char* _shaderRootPath;

int game_init(const char* gameShaderRootPath) {
    size_t n = strlen(gameShaderRootPath);
    if (!(_shaderRootPath = malloc(++n))) {
        return 0;
    }
    memcpy(_shaderRootPath, gameShaderRootPath, n);
    shaderRootPath = _shaderRootPath;

    random_seed(4357);

    return 1;
}

void game_free(void) {
    free(_shaderRootPath);
}
