#include <game/render/shader.h>
#include <game/render/vertex_array.h>

#ifndef GAME_RENDER_VERTEX_SHADER_H
#define GAME_RENDER_VERTEX_SHADER_H

void vertex_standard_load_skinned(GLuint program, void* params);

GLuint vertex_shader_standard(enum MeshFlags flags);
GLuint vertex_shader_overlay(enum MeshFlags flags);

#endif
