#include <stdlib.h>
#include <game/material/solid_text.h>
#include "shaders.h"

static void text_load(const struct Material* material, const struct Camera* camera, const struct Lights* lights) {
    glUniform3fv(glGetUniformLocation(material->shader, "textColor"), 1, ((struct SolidTextMaterial*)material)->color);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, ((struct SolidTextMaterial*)material)->font->texture_atlas);
    glUniform1i(glGetUniformLocation(material->shader, "tex"), 0);
}

struct SolidTextMaterial* solid_text_material_new(float r, float g, float b, const struct BitmapFont* font) {
    static const char* defines[] = {"HAVE_TEXCOORD", NULL};
    struct SolidTextMaterial* solidText;

    if (!game_shaders[SHADER_SOLID_TEXT]) {
        if (!(game_shaders[SHADER_SOLID_TEXT] = game_load_shader("standard.vert", "text.frag", defines, sizeof(defines) / (2 * sizeof(*defines))))) {
            return NULL;
        }
    }
    if (!(solidText = malloc(sizeof(*solidText)))) {
        return NULL;
    }
    solidText->material.load = text_load;
    solidText->material.shader = game_shaders[SHADER_SOLID_TEXT];
    solidText->material.polygonMode = GL_FILL;
    solidText->color[0] = r;
    solidText->color[1] = g;
    solidText->color[2] = b;
    solidText->font = font;

    return solidText;
}
