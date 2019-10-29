#include <game/init.h>
#include <game/render/vertex_shader.h>
#include <game/render/shader.h>

GLuint vertex_shader_standard(enum MeshFlags flags) {
    const char* defines[2 * 3];
    unsigned int numDefines = 0;

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
    return shader_find_compile("standard.vert", GL_VERTEX_SHADER, &shaderRootPath, 1, defines, numDefines);
}
