#include <game/render/globject.h>

#ifndef SKYBOX_H
#define SKYBOX_H

GLuint skybox_load_texture_png_6faces(const char* left, const char* right, const char* bottom, const char* top, const char* back, const char* front);
GLuint skybox_load_texture_hdr_equirect(const char* path, unsigned int cubeFaceSize);

int skybox_create(GLuint texture, float size, struct GLObject* skybox);
void skybox_destroy(struct GLObject* skybox);

struct GLObject* skybox_new(GLuint texture, float size);
void skybox_free(struct GLObject* skybox);

void skybox_render(struct GLObject* skybox, const struct Camera* camera);

#endif
