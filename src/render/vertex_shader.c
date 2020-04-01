#include <game/init.h>
#include <game/render/vertex_shader.h>
#include <game/render/shader.h>
#include <game/render/vertex_array.h>
#include <game/scene/skin.h>

#define _STR(n) #n
#define NB_STR(n) _STR(n)
#define MAX_NB_BONES_STR NB_STR(MAX_NB_BONES)

GLuint vertex_shader_standard_(enum MeshFlags flags, int overlay) {
    const char* defines[2 * 7];
    unsigned int numDefines = 0;

    if (overlay) {
        defines[2 * numDefines] = "OVERLAY";
        defines[2 * numDefines++ + 1] = NULL;
    }
    if (flags & MESH_NORMALS) {
        defines[2 * numDefines] = "HAVE_NORMAL";
        defines[2 * numDefines++ + 1] = NULL;
    }
    if (flags & MESH_TEXCOORDS) {
        defines[2 * numDefines] = "HAVE_TEXCOORD";
        defines[2 * numDefines++ + 1] = NULL;
    }
    if (flags & MESH_TANGENTS) {
        defines[2 * numDefines] = "HAVE_TANGENT";
        defines[2 * numDefines++ + 1] = NULL;
    }
    if (flags & MESH_SKIN) {
        defines[2 * numDefines] = "HAVE_SKIN";
        defines[2 * numDefines++ + 1] = NULL;
        defines[2 * numDefines] = "NB_BONES";
        defines[2 * numDefines++ + 1] = MAX_NB_BONES_STR;
    }
    return shader_find_compile("standard.vert", GL_VERTEX_SHADER, &shaderRootPath, 1, defines, numDefines);
}

GLuint vertex_shader_standard(enum MeshFlags flags) {
    return vertex_shader_standard_(flags, 0);
}

GLuint vertex_shader_overlay(enum MeshFlags flags) {
    return vertex_shader_standard_(flags, 1);
}
