#include <GL/glew.h>

#ifndef ASSETS_H
#define ASSETS_H

extern char* shaderRootPath;

GLuint game_load_shader(const char* vertexShaderFilename, const char* fragmentShaderFilename, const char** defines, size_t numDefines);

#endif
