#include <GL/glew.h>

#ifndef ASSETS_H
#define ASSETS_H

enum Shader {
    SHADER_SOLID_COLOR,
    SHADER_SOLID_TEXTURE,
    SHADER_PHONG_COLOR,
    SHADER_PHONG_TEXTURE,
    NUM_SHADERS
};

extern GLuint* game_shaders;

int game_load_shaders(GLuint* shaders);
void game_free_shaders(GLuint* shaders);

#endif
