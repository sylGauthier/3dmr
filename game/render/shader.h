#include <stdio.h>
#include <GL/glew.h>

#ifndef SHADER_H
#define SHADER_H

#define LOCATION_VERTEX 0
#define LOCATION_NORMAL 1
#define LOCATION_TEXCOORD 2
#define LOCATION_TANGENT 3
#define LOCATION_BITANGENT 4

#define CAMERA_UBO_BINDING 0
#define LIGHTS_UBO_BINDING 1

GLuint shader_find_compile(const char* filename, GLenum type, const char** includePaths, size_t numIncludePaths, const char** defines, size_t numDefines);
GLuint shader_compile(const char* path, GLenum type, const char** includePaths, size_t numIncludePaths, const char** defines, size_t numDefines);
GLuint shader_compile_fd(FILE* fd, const char* pathInfo, GLenum type, const char** includePaths, size_t numIncludePaths, const char** defines, size_t numDefines);
GLuint shader_link(const GLuint* shaders, size_t numShaders);
    
#endif
