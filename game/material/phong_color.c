#include <stdlib.h>
#include "solid_color.h"
#include "phong_color.h"
#include "shaders.h"

static void phong_color_prerender(const struct Material* material, const struct Camera* camera, const struct Lights* lights) {
    solid_color_load_uniform(material, ((const struct PhongColorMaterial*)material)->color);
    light_load_uniforms(material->shader, lights);
    phong_material_load_uniform(material->shader, &((const struct PhongColorMaterial*)material)->phong);
}

struct PhongColorMaterial* phong_color_material_new(float r, float g, float b, const struct PhongMaterial* phong) {
    struct PhongColorMaterial* phongColor;

    if (!(phongColor = malloc(sizeof(*phongColor)))) {
        return NULL;
    }
    phongColor->material.prerender = phong_color_prerender;
    phongColor->material.postrender = NULL;
    phongColor->material.shader = game_shaders[SHADER_PHONG_COLOR];
    phongColor->material.polygonMode = GL_FILL;
    phongColor->phong = *phong;
    phongColor->color[0] = r;
    phongColor->color[1] = g;
    phongColor->color[2] = b;

    return phongColor;
}
