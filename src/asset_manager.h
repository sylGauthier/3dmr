#ifndef ASSET_MANAGER_H
#define ASSET_MANAGER_H

#include <GL/glew.h>
#include "globject.h"

#include "assets.h"

GLuint load_texture(enum Texture key);

GLuint load_shader(enum Shader key);

struct GLObject load_model(enum Model key);

int assets_init();

void assets_free();

#endif
