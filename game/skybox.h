#include <game/render/material.h>
#include <game/render/vertex_array.h>

#ifndef SKYBOX_H
#define SKYBOX_H

struct Skybox {
    struct Material material;
    GLuint texture;
    struct VertexArray vertexArray;
};

GLuint skybox_load_texture_png_6faces(const char* left, const char* right, const char* bottom, const char* top, const char* back, const char* front);
GLuint skybox_load_texture_hdr_equirect(const char* path, unsigned int cubeFaceSize);

int skybox_create(GLuint texture, float size, struct Skybox* skybox);
void skybox_destroy(struct Skybox* skybox);

struct Skybox* skybox_new(GLuint texture, float size);
void skybox_free(struct Skybox* skybox);

void skybox_render(struct Skybox* skybox);

#endif
