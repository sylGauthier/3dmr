#include <stdio.h>
#include <GL/glew.h>

#ifndef TDMR_RENDER_SHADER_H
#define TDMR_RENDER_SHADER_H

#define LOCATION_VERTEX 0
#define LOCATION_NORMAL 1
#define LOCATION_TEXCOORD 2
#define LOCATION_TANGENT 3
#define LOCATION_BITANGENT 4
#define LOCATION_BONE_IDX 5
#define LOCATION_BONE_WEIGHT 6

#define CAMERA_UBO_BINDING 0
#define LIGHTS_UBO_BINDING 1
#define BONES_UBO_BINDING 2

#define TEX_SLOT_COLOR_SOLID    0

#define TEX_SLOT_COLOR_DIFFUSE  0
#define TEX_SLOT_COLOR_AMBIENT  1
#define TEX_SLOT_COLOR_SPECULAR 2
#define TEX_SLOT_SHININESS      3

#define TEX_SLOT_COLOR_ALBEDO   0
#define TEX_SLOT_ROUGHNESS      1
#define TEX_SLOT_METALNESS      2
#define TEX_SLOT_MRMIXED        1

#define TEX_SLOT_IBL_IRRADIANCE 3
#define TEX_SLOT_IBL_SPECULAR   4
#define TEX_SLOT_IBL_SPECBRDF   5

#define TEX_SLOT_COLOR_ALPHA    6
#define TEX_SLOT_NORMALMAP      7
#define TEX_SLOT_OCCLUSIONMAP   8
#define TEX_SLOT_DIR_SHADOWMAP  9

GLuint shader_find_compile(const char* filename, GLenum type, const char** includePaths, size_t numIncludePaths, const char** defines, size_t numDefines);
GLuint shader_compile(const char* path, GLenum type, const char** includePaths, size_t numIncludePaths, const char** defines, size_t numDefines);
GLuint shader_compile_fd(FILE* fd, const char* pathInfo, GLenum type, const char** includePaths, size_t numIncludePaths, const char** defines, size_t numDefines);
GLuint shader_link(const GLuint* shaders, size_t numShaders);
GLuint shader_find_compile_link_vertfrag(const char* vpath, const char* fpath, const char** includePaths, size_t numIncludePaths, const char** vdefines, size_t vNumDefines, const char** fdefines, size_t fNumDefines);
    
#endif
