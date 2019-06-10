#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/glew.h>
#include <game/render/shader.h>

GLuint* game_shaders;
char* shaderRootPath;

int game_load_shader(const char* vertexShaderFilename, const char* fragmentShaderFilename, const char** defines, size_t numDefines) {
    GLuint res = 0;
    char *vertexShaderPath, *fragmentShaderPath;

    if (!shaderRootPath) {
        fprintf(stderr, "Error: shader root path not set\n");
        return 0;
    }
    vertexShaderPath = malloc(strlen(shaderRootPath) + strlen(vertexShaderFilename) + 2);
    fragmentShaderPath = malloc(strlen(shaderRootPath) + strlen(fragmentShaderFilename) + 2);
    if (!vertexShaderPath || !fragmentShaderPath) {
        fprintf(stderr, "Error: memory allocation failed\n");
    } else {
        sprintf(vertexShaderPath, "%s/%s", shaderRootPath, vertexShaderFilename);
        sprintf(fragmentShaderPath, "%s/%s", shaderRootPath, fragmentShaderFilename);
        res = shader_compile_link_vert_frag(vertexShaderPath, fragmentShaderPath, (const char**)&shaderRootPath, 1, defines, numDefines);
    }

    free(vertexShaderPath);
    free(fragmentShaderPath);
    return res;
}

