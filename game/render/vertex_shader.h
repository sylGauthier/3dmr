#include <game/render/shader.h>
#include <game/mesh/mesh.h>

#ifndef GAME_RENDER_VERTEX_SHADER_H
#define GAME_RENDER_VERTEX_SHADER_H

GLuint vertex_shader_standard(enum MeshFlags flags);
GLuint vertex_shader_overlay(enum MeshFlags flags);

#endif
