#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "asset_manager.h"
#include "shader.h"
#include "geometry/shaders.h"

#define MAX_SHADER_NAME_SIZE 128
struct ShaderFilename {
    const char* vertexShaderFilename;
    const char* fragmentShaderFilename;
} shaderFilenames[NUM_SHADERS] = {
    {"solid_color.vert", "solid_color.frag"},
    {"solid_texture.vert", "solid_texture.frag"},
    {"phong_color.vert", "phong_color.frag"},
    {"phong_texture.vert", "phong_texture.frag"}
};

GLuint* game_shaders;

int game_load_shaders(GLuint* shaders) {
    enum Shader shader;

    for (shader = 0; shader < NUM_SHADERS; shader++) {
        if (!(shaders[shader] = asset_manager_load_shader(shaderFilenames[shader].vertexShaderFilename, shaderFilenames[shader].fragmentShaderFilename))) {
            break;
        }
    }
    if (shader == NUM_SHADERS) {
        return 1;
    }

    for (; shader < NUM_SHADERS; shader++) {
        shaders[shader] = 0;
    }
    game_free_shaders(shaders);
    return 0;
}

void game_free_shaders(GLuint* shaders) {
    enum Shader shader;

    for (shader = 0; shader < NUM_SHADERS; shader++) {
        if (shaders[shader]) {
            glDeleteProgram(shaders[shader]);
            shaders[shader] = 0;
        }
    }
}
