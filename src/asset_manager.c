#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <game/render/shader.h>
#include <game/render/texture.h>

#define MAX_NUM_PATHS 128

static char* paths[MAX_NUM_PATHS];
static unsigned int numPaths = 0;
static size_t maxPathSize = 0;

int asset_manager_add_path(const char* path) {
    size_t pathSize;
    if (numPaths >= MAX_NUM_PATHS || !(paths[numPaths] = malloc(strlen(path) + 1))) {
        return 0;
    }
    strcpy(paths[numPaths++], path);
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
            free(paths[i]);
            paths[i] = paths[--numPaths];
        }
    }
}

size_t asset_manager_get_num_paths(void) {
    return numPaths;
}

const char* asset_manager_get_path(size_t i) {
    return paths[i];
}

char* asset_manager_find_file(const char* filename) {
    unsigned int i;
    size_t filenameSize = strlen(filename);
    char* path;
    FILE* test;

    if (!(path = malloc(maxPathSize + 1 + filenameSize + 1))) {
        return 0;
    }
    for (i = 0; i < numPaths; i++) {
        sprintf(path, "%s/%s", paths[i], filename);
        if ((test = fopen(path, "r"))) {
            fclose(test);
            return path;
        }
    }
    free(path);
    return NULL;
}

GLuint asset_manager_load_shader(const char* vertexShaderFilename, const char* fragmentShaderFilename) {
    GLuint res = 0;
    char* vertexShaderPath = asset_manager_find_file(vertexShaderFilename);
    char* fragmentShaderPath = asset_manager_find_file(fragmentShaderFilename);

    if (!vertexShaderPath || !fragmentShaderPath) {
        if (!vertexShaderPath) fprintf(stderr, "Error: failed to find shader source '%s'\n", vertexShaderFilename);
        if (!fragmentShaderPath) fprintf(stderr, "Error: failed to find shader source '%s'\n", fragmentShaderFilename);
    } else {
        res = shader_compile_link_vert_frag(vertexShaderPath, fragmentShaderPath, NULL, 0);
    }

    free(vertexShaderPath);
    free(fragmentShaderPath);
    return res;
}

GLuint asset_manager_load_texture(const char* filename) {
    GLuint res = 0;
    char* path = asset_manager_find_file(filename);

    if (!path) {
        fprintf(stderr, "Error: failed to find texture '%s'\n", filename);
    } else {
        res = texture_load_from_file(path);
    }

    free(path);
    return res;
}

void asset_manager_free(void) {
    while (numPaths) {
        free(paths[--numPaths]);
    }
}
