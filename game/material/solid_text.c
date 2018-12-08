#include <stdlib.h>
#include "solid_text.h"
#include "shaders.h"

static void text_prerender(const struct Material* material, const struct Camera* camera, const struct Lights* lights) {
    glUniform3fv(glGetUniformLocation(material->shader, "textColor"), 1, ((struct SolidTextMaterial*)material)->color);
    glBindTexture(GL_TEXTURE_2D, ((struct SolidTextMaterial*)material)->font->texture_atlas);
}

static void text_postrender(const struct Material* material, const struct Camera* camera, const struct Lights* lights) {
    glBindTexture(GL_TEXTURE_2D, 0);
}


struct SolidTextMaterial* solid_text_material_new(float r, float g, float b, const struct BitmapFont* font) {
    struct SolidTextMaterial* solidText;

    if (!(solidText = malloc(sizeof(*solidText)))) {
        return NULL;
    }
    solidText->material.prerender = text_prerender;
    solidText->material.postrender = text_postrender;
    solidText->material.shader = game_shaders[SHADER_SOLID_TEXT];
    solidText->material.polygonMode = GL_FILL;
    solidText->color[0] = r;
    solidText->color[1] = g;
    solidText->color[2] = b;
    solidText->font = font;

    return solidText;
}
