#include <3dmr/render/material.h>
#include <3dmr/render/vertex_array.h>

#ifndef TDMR_SKYBOX_H
#define TDMR_SKYBOX_H

struct Skybox {
    struct Material material;
    GLuint texture;
    struct VertexArray vertexArray;
};

GLuint skybox_load_texture_png_6faces(const char* left, const char* right, const char* bottom, const char* top, const char* back, const char* front);
GLuint skybox_load_texture_hdr_equirect(const char* path, unsigned int cubeFaceSize);
GLuint skybox_load_texture_hdr_equirect_32(const char* path, unsigned int cubeFaceSize);

int skybox_create(GLuint texture, struct Skybox* skybox);
void skybox_destroy(struct Skybox* skybox);

struct Skybox* skybox_new(GLuint texture);
void skybox_free(struct Skybox* skybox);

void skybox_render(struct Skybox* skybox);

#endif
