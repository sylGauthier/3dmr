#include <GL/glew.h>

#ifndef ASSETS_H
#define ASSETS_H

enum Shader {
    SHADER_SOLID_COLOR,
    SHADER_SOLID_TEXTURE,
    SHADER_PHONG_COLOR,
    SHADER_PHONG_TEXTURE,
    SHADER_PHONG_TEXTURE_NORMALMAP,
    SHADER_PBR,
    SHADER_PBR_UNI,
    SHADER_SKYBOX,
    NUM_SHADERS
};

extern GLuint* game_shaders;
extern char* shaderRootPath;

int game_load_shader(const char* vertexShaderFilename, const char* fragmentShaderFilename, const char** defines, size_t numDefines);

#endif
