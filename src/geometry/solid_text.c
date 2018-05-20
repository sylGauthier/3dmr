#include <stdlib.h>
#include "solid_text.h"
#include "shaders.h"

static void text_prerender(const struct Geometry* geometry, const struct Camera* camera, const struct Lights* lights) {
    struct SolidTextMaterial* material = geometry->material;
    glUniform3fv(glGetUniformLocation(geometry->shader, "textColor"), 1, material->color);
    glBindTexture(GL_TEXTURE_2D, material->font->texture_atlas);
}

static void text_postrender(const struct Geometry* geometry, const struct Camera* camera, const struct Lights* lights) {
    glBindTexture(GL_TEXTURE_2D, 0);
}


static void solid_text_geometry_init(struct Geometry* dest, const struct GLObject* glObject) {
    dest->glObject = *glObject;
    dest->shader = game_shaders[SHADER_SOLID_TEXT];
    dest->mode = GL_FILL;
    dest->prerender = text_prerender;
    dest->postrender = text_postrender;
}

struct Geometry* solid_text_geometry(const struct GLObject* glObject, float r, float g, float b, const struct BitmapFont* font) {
    struct Geometry* dest;
    struct SolidTextMaterial* material;

    if (!(dest = malloc(sizeof(struct Geometry) + sizeof(struct SolidTextMaterial)))) {
        return NULL;
    }

    solid_text_geometry_init(dest, glObject);

    material = dest->material = dest + 1;
    material->color[0] = r;
    material->color[1] = g;
    material->color[2] = b;
    material->font = font;

    return dest;
}

struct Geometry* solid_text_geometry_shared(const struct GLObject* glObject, struct SolidTextMaterial* material) {
    struct Geometry* dest;

    if (!(dest = malloc(sizeof(struct Geometry)))) {
        return NULL;
    }

    solid_text_geometry_init(dest, glObject);

    dest->material = material;

    return dest;
}
