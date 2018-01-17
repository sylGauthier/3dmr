#include <stdlib.h>
#include "phong_texture.h"
#include "shader.h"

static GLuint shader = 0;

static void phong_texture_prerender(const struct Geometry* geometry, const struct Camera* camera, const struct Lights* lights) {
    glBindTexture(GL_TEXTURE_2D, ((const struct PhongTextureMaterial*)geometry->material)->texture);
    light_load_uniforms(geometry->shader, lights->directional, lights->numDirectional, lights->local, lights->numLocal);
    phong_load_material_uniform(geometry->shader, &((const struct PhongTextureMaterial*)geometry->material)->phong);
}

static void phong_texture_postrender(const struct Geometry* geometry, const struct Camera* camera, const struct Lights* lights) {
    glBindTexture(GL_TEXTURE_2D, 0);
}

static void phong_texture_geometry_init(struct Geometry* dest, const struct GLObject* glObject) {
    dest->glObject = *glObject;
    if (!shader) {
        shader = shader_compile("shaders/phong_texture.vert", "shaders/phong_texture.frag");
    }
    dest->shader = shader;
    dest->mode = GL_FILL;
    dest->prerender = phong_texture_prerender;
    dest->postrender = phong_texture_postrender;
}

struct Geometry* phong_texture_geometry(const struct GLObject* glObject, GLuint texture, const struct PhongMaterial* phong) {
    struct Geometry* dest;
    struct PhongTextureMaterial* material;

    if (!(dest = malloc(sizeof(struct Geometry) + sizeof(struct PhongTextureMaterial)))) {
        return NULL;
    }

    phong_texture_geometry_init(dest, glObject);

    material = dest->material = dest + 1;
    material->texture = texture;
    material->phong = *phong;

    return dest;
}

struct Geometry* phong_texture_geometry_shared(const struct GLObject* glObject, struct PhongTextureMaterial* material) {
    struct Geometry* dest;

    if (!(dest = malloc(sizeof(struct Geometry)))) {
        return NULL;
    }

    phong_texture_geometry_init(dest, glObject);

    dest->material = material;

    return dest;
}

void phong_texture_shader_free(void) {
    glDeleteProgram(shader);
}
