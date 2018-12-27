#include <game/render/globject.h>

#ifndef SKYBOX_H
#define SKYBOX_H

GLuint skybox_load_texture(const char* left, const char* right, const char* bottom, const char* top, const char* back, const char* front);

int skybox_create(GLuint texture, float size, struct GLObject* skybox);
void skybox_destroy(struct GLObject* skybox);

struct GLObject* skybox_new(GLuint texture, float size);
void skybox_free(struct GLObject* skybox);

#endif
