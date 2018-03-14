#include "shader.h"

#ifndef ASSET_MANAGER_H
#define ASSET_MANAGER_H

int asset_manager_add_path(const char* path);
void asset_manager_remove_path(const char* path);

char* asset_manager_find_shader(const char* filename);
GLuint asset_manager_load_shader(const char* vertexShaderFilename, const char* fragmentShaderFilename);

#endif
