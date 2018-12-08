#include <stdlib.h>
#include "solid_color.h"
#include "shaders.h"

void solid_color_load_uniform(const struct Material* material, const float* color) {
    glUniform3fv(glGetUniformLocation(material->shader, "solidColor"), 1, color);
}

static void solid_color_prerender(const struct Material* material, const struct Camera* camera, const struct Lights* lights) {
    solid_color_load_uniform(material, ((const struct SolidColorMaterial*)material)->color);
}

struct SolidColorMaterial* solid_color_material_new(float r, float g, float b) {
    struct SolidColorMaterial* solidColor;

    if (!(solidColor = malloc(sizeof(*solidColor)))) {
        return NULL;
    }
    solidColor->material.prerender = solid_color_prerender;
    solidColor->material.postrender = NULL;
    solidColor->material.shader = game_shaders[SHADER_SOLID_COLOR];
    solidColor->material.polygonMode = GL_FILL;
    solidColor->color[0] = r;
    solidColor->color[1] = g;
    solidColor->color[2] = b;

    return solidColor;
}
