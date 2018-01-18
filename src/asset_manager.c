#include <GL/glew.h>
#include <stdlib.h>

#include "asset_manager.h"
#include "mesh/obj.h"
#include "shader.h"
#include "texture.h"
#define DEBUG
#include "debug.h"

struct GLObject models[NB_MODELS + 1] = {0};

struct Mesh meshs[NB_MODELS + 1] = {0};

GLuint textures[NB_TEXTURES + 1] = {0};

GLuint shaders[NB_SHADERS + 1] = {0};


struct GLObject load_model(enum Model key)
{
    return models[key];
}

GLuint load_texture(enum Texture key)
{
    return textures[key];
}

GLuint load_shader(enum Shader key)
{
    return shaders[key];
}

int models_init()
{
    const struct Model_key *key;
    unsigned int i;

    for (i = 0; i < NB_MODELS; i++) {
        key = &model_keys[i];
        dbg_printf("models init: [%d/%d] %s\n", i, NB_MODELS, key->path);

        make_obj(&meshs[i], key->path, key->withIndices,
              key->withNormals, key->withTexCoords);
        globject_new(&meshs[i], &models[i]);
    }
    dbg_printf("models init: [%d/%1$d] ... DONE\n", NB_MODELS);

    return 0;
}

int shaders_init()
{
    const struct Shader_key *key;
    unsigned int i;

    for (i = 0; i < NB_SHADERS; i++) {
        key = &shader_keys[i];
        dbg_printf("shaders init: [%d/%d] %s - %s\n", i, NB_SHADERS,
                   key->vert, key->frag);

        shaders[i] = shader_compile(key->vert, key->frag);

        if (!shaders[i]) {
            return 1;
        }
    }
    dbg_printf("shaders init: [%d/%1$d] ... DONE\n", NB_SHADERS);

    return 0;
}

int textures_init()
{
    unsigned int i;

    for (i = 0; i < NB_TEXTURES; i++) {
        dbg_printf("textures init: [%d/%d] %s\n", i, NB_TEXTURES,
            texture_keys[i]);
        textures[i] = texture_load_from_file(texture_keys[i]);
    }
    dbg_printf("textures init: [%d/%1$d] ... DONE\n", NB_TEXTURES);

    return 0;
}

int assets_init()
{
    int err;

    err = models_init();
    if (err) {
        dbg_printf("models initialisation failed.");
        assets_free();
        return err;
    }

    err = shaders_init();
    if (err) {
        dbg_printf("shaders initialisation failed.");
        assets_free();
        return err;
    }

    err = textures_init();
    if (err) {
        dbg_printf("textures initialisation failed.");
        assets_free();
        return err;
    }

    return 0;
}

void assets_free()
{
    unsigned int i;

    for (i = 0; i < NB_MODELS; i++) {
        globject_free(&models[i]);
        mesh_free(&meshs[i]);
    }

    for (i = 0; i < NB_SHADERS; i++) {
        if (shaders[i])
            glDeleteProgram(shaders[i]);
        else
            break;
    }

    glDeleteTextures(NB_TEXTURES, textures);
}
