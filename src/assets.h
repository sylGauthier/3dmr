#ifndef ASSETS_H
#define ASSETS_H

enum Model {
    CUBE,
    NB_MODELS
};

enum Texture {
    TUX,
    NB_TEXTURES
};

enum Shader {
    SOLID_COLOR,
    NB_SHADERS
};


struct Model_key {
    const char *path;
    const int withIndices;
    const int withNormals;
    const int withTexCoords;
};

struct Shader_key {
    const char *vert;
    const char *frag;
};

static const struct Model_key model_keys[NB_MODELS + 1] =
{
    { "models/cube.obj", 0, 0, 1 },
    { 0 }
};

static const char *const texture_keys[NB_TEXTURES + 1] =
{
    "textures/tux.png",
    0
};

static const struct Shader_key shader_keys[NB_SHADERS + 1] =
{
    { "shaders/solid_color.vert", "shaders/solid_color.frag" },
    { 0 }
};


#endif
