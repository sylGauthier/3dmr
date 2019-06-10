#include <stdio.h>
#include <GL/glew.h>

#ifndef SHADER_H
#define SHADER_H

#define LOCATION_VERTEX 0
#define LOCATION_NORMAL 1
#define LOCATION_TEXCOORD 2
#define LOCATION_TANGENT 3
#define LOCATION_BITANGENT 4

GLuint shader_compile(const char* path, GLenum type, const char** includePaths, size_t numIncludePaths, const char** defines, size_t numDefines);
GLuint shader_compile_fd(FILE* fd, const char* pathInfo, GLenum type, const char** includePaths, size_t numIncludePaths, const char** defines, size_t numDefines);
GLuint shader_link(GLuint* shaders, size_t numShaders);
    
GLuint shader_compile_link_vert_frag(const char* vertexShaderPath, const char* fragmentShaderPath, const char** includePaths, size_t numIncludePaths, const char** defines, size_t numDefines);

#endif
