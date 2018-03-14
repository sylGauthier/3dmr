#include <stdlib.h>
#include "solid_texture.h"
#include "shaders.h"

static void solid_texture_prerender(const struct Geometry* geometry, const struct Camera* camera, const struct Lights* lights) {
    glBindTexture(GL_TEXTURE_2D, ((const struct SolidTextureMaterial*)geometry->material)->texture);
}

static void solid_texture_postrender(const struct Geometry* geometry, const struct Camera* camera, const struct Lights* lights) {
    glBindTexture(GL_TEXTURE_2D, 0);
}

static void solid_texture_geometry_init(struct Geometry* dest, const struct GLObject* glObject) {
    dest->glObject = *glObject;
    dest->shader = game_shaders[SHADER_SOLID_TEXTURE];
    dest->mode = GL_FILL;
    dest->prerender = solid_texture_prerender;
    dest->postrender = solid_texture_postrender;
}

struct Geometry* solid_texture_geometry(const struct GLObject* glObject, GLuint texture) {
    struct Geometry* dest;
    struct SolidTextureMaterial* material;

    if (!(dest = malloc(sizeof(struct Geometry) + sizeof(struct SolidTextureMaterial)))) {
        return NULL;
    }

    solid_texture_geometry_init(dest, glObject);

    material = dest->material = dest + 1;
    material->texture = texture;

    return dest;
}

struct Geometry* solid_texture_geometry_shared(const struct GLObject* glObject, struct SolidTextureMaterial* material) {
    struct Geometry* dest;

    if (!(dest = malloc(sizeof(struct Geometry)))) {
        return NULL;
    }

    solid_texture_geometry_init(dest, glObject);

    dest->material = material;

    return dest;
}
