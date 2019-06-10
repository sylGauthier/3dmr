#include <stddef.h>
#include <GL/glew.h>

#ifndef ASSET_MANAGER_H
#define ASSET_MANAGER_H

int asset_manager_add_path(const char* path);
void asset_manager_remove_path(const char* path);

size_t asset_manager_get_num_paths(void);
const char* asset_manager_get_path(size_t i);

char* asset_manager_find_file(const char* filename);
GLuint asset_manager_load_texture(const char* filename);

void asset_manager_free(void);

#endif
