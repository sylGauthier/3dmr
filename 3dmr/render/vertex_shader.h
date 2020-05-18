#include <3dmr/render/shader.h>
#include <3dmr/render/vertex_array.h>

#ifndef TDMR_RENDER_VERTEX_SHADER_H
#define TDMR_RENDER_VERTEX_SHADER_H

GLuint vertex_shader_standard(enum MeshFlags flags);
GLuint vertex_shader_overlay(enum MeshFlags flags);

#endif
