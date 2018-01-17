#include <stdlib.h>
#include "solid_color.h"
#include "phong_color.h"
#include "shader.h"

static GLuint shader = 0;

static void load_phong_color_uniforms(const struct Geometry* geometry, const struct Camera* camera, const struct Lights* lights) {
    solid_color_load_uniform(geometry, ((const struct PhongColorMaterial*)geometry->material)->color);
    light_load_uniforms(geometry->shader, lights->directional, lights->numDirectional, lights->local, lights->numLocal);
    phong_load_material_uniform(geometry->shader, &((const struct PhongColorMaterial*)geometry->material)->phong);
}

static void phong_color_geometry_init(struct Geometry* dest, const struct GLObject* glObject) {
    dest->glObject = *glObject;
    if (!shader) {
        shader = shader_compile("shaders/phong_color.vert", "shaders/phong_color.frag");
    }
    dest->shader = shader;
    dest->mode = GL_FILL;
    dest->prerender = load_phong_color_uniforms;
    dest->postrender = 0;
}

struct Geometry* phong_color_geometry(const struct GLObject* glObject, float r, float g, float b, const struct PhongMaterial* phong) {
    struct Geometry* dest;
    struct PhongColorMaterial* material;

    if (!(dest = malloc(sizeof(struct Geometry) + sizeof(struct PhongColorMaterial)))) {
        return NULL;
    }

    phong_color_geometry_init(dest, glObject);

    material = dest->material = dest + 1;
    material->color[0] = r;
    material->color[1] = g;
    material->color[2] = b;
    material->phong = *phong;

    return dest;
}

struct Geometry* phong_color_geometry_shared(const struct GLObject* glObject, struct PhongColorMaterial* material) {
    struct Geometry* dest;

    if (!(dest = malloc(sizeof(struct Geometry)))) {
        return NULL;
    }

    phong_color_geometry_init(dest, glObject);

    dest->material = material;

    return dest;
}

void phong_color_shader_free(void) {
    glDeleteProgram(shader);
}
