#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "shader.h"

#define MAX_NUM_PATHS 128

static const char* paths[MAX_NUM_PATHS];
static unsigned int numPaths = 0;
static size_t maxPathSize = 0;

int asset_manager_add_path(const char* path) {
    size_t pathSize;
    if (numPaths >= MAX_NUM_PATHS) {
        return 0;
    }
    paths[numPaths++] = path;
    if (maxPathSize < (pathSize = strlen(path))) {
        maxPathSize = pathSize;
    }
    return 1;
}

void asset_manager_remove_path(const char* path) {
    unsigned int i;
    for (i = 0; i < numPaths;) {
        if (strcmp(paths[i], path)) {
            i++;
        } else {
            paths[i] = paths[--numPaths];
        }
    }
}

static char* asset_manager_find_file(const char* type, const char* filename) {
    unsigned int i;
    size_t filenameSize = strlen(filename);
    size_t typeSize = strlen(type);
    char* path;
    FILE* test;

    if (!(path = malloc(maxPathSize + 1 + typeSize + 1 + filenameSize + 1))) {
        return 0;
    }
    for (i = 0; i < numPaths; i++) {
        sprintf(path, "%s/%s/%s", paths[i], type, filename);
        if ((test = fopen(path, "r"))) {
            fclose(test);
            return path;
        }
    }
    free(path);
    return NULL;
}

char* asset_manager_find_shader(const char* filename) {
    return asset_manager_find_file("shaders", filename);
}

GLuint asset_manager_load_shader(const char* vertexShaderFilename, const char* fragmentShaderFilename) {
    GLuint res = 0;
    char* vertexShaderPath = asset_manager_find_shader(vertexShaderFilename);
    char* fragmentShaderPath = asset_manager_find_shader(fragmentShaderFilename);

    if (!vertexShaderPath || !fragmentShaderPath) {
        if (!vertexShaderPath) fprintf(stderr, "Error: failed to find shader source '%s'\n", vertexShaderFilename);
        if (!fragmentShaderPath) fprintf(stderr, "Error: failed to find shader source '%s'\n", fragmentShaderFilename);
    } else {
        res = shader_compile(vertexShaderPath, fragmentShaderPath);
    }

    free(vertexShaderPath);
    free(fragmentShaderPath);
    return res;
}
