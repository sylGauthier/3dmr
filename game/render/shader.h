#include <stdio.h>
#include <GL/glew.h>

#ifndef SHADER_H
#define SHADER_H

#define LOCATION_VERTEX 0
#define LOCATION_NORMAL 1
#define LOCATION_TEXCOORD 2

GLuint shader_compile(const char* path, GLenum type);
GLuint shader_compile_fd(FILE* fd, const char* pathInfo, GLenum type);
GLuint shader_link(GLuint* shaders, size_t numShaders);
    
GLuint shader_compile_link_vert_frag(const char* vertexShaderPath, const char* fragmentShaderPath);

#endif
