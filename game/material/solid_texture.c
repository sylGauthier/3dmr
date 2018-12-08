#include <stdlib.h>
#include "solid_texture.h"
#include "shaders.h"

static void solid_texture_prerender(const struct Material* material, const struct Camera* camera, const struct Lights* lights) {
    glBindTexture(GL_TEXTURE_2D, ((const struct SolidTextureMaterial*)material)->texture);
}

static void solid_texture_postrender(const struct Material* material, const struct Camera* camera, const struct Lights* lights) {
    glBindTexture(GL_TEXTURE_2D, 0);
}

struct SolidTextureMaterial* solid_texture_material_new(GLuint texture) {
    struct SolidTextureMaterial* solidTexture;

    if (!(solidTexture = malloc(sizeof(*solidTexture)))) {
        return NULL;
    }
    solidTexture->material.prerender = solid_texture_prerender;
    solidTexture->material.postrender = solid_texture_postrender;
    solidTexture->material.shader = game_shaders[SHADER_SOLID_TEXTURE];
    solidTexture->material.polygonMode = GL_FILL;
    solidTexture->texture = texture;

    return solidTexture;
}
