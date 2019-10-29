#ifndef GAME_INIT_H
#define GAME_INIT_H

extern const char* shaderRootPath;

int game_init(const char* gameShaderRootPath);
void game_free(void);

#endif
