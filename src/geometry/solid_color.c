#include <stdlib.h>
#include "solid_color.h"
#include "shader.h"

static GLuint shader = 0;

static void load_solid_color_uniforms(const struct Geometry* geometry, const struct Camera* camera) {
    glUniform3fv(glGetUniformLocation(geometry->shader, "solidColor"), 1, (float*)((const struct SolidColorMaterial*)geometry->material)->color);
}

static void solid_color_geometry_init(struct Geometry* dest, const struct GLObject* glObject) {
    dest->glObject = *glObject;
    if (!shader) {
        shader = shader_compile("shaders/solid_color.vert", "shaders/solid_color.frag");
    }
    dest->shader = shader;
    dest->mode = GL_FILL;
    dest->prerender = load_solid_color_uniforms;
    dest->postrender = 0;
}

struct Geometry* solid_color_geometry(const struct GLObject* glObject, float r, float g, float b) {
    struct Geometry* dest;
    struct SolidColorMaterial* material;

    if (!(dest = malloc(sizeof(struct Geometry) + sizeof(struct SolidColorMaterial)))) {
        return NULL;
    }

    solid_color_geometry_init(dest, glObject);

    material = dest->material = dest + 1;
    material->color[0] = r;
    material->color[1] = g;
    material->color[2] = b;

    return dest;
}

struct Geometry* solid_color_geometry_shared(const struct GLObject* glObject, struct SolidColorMaterial* material) {
    struct Geometry* dest;

    if (!(dest = malloc(sizeof(struct Geometry)))) {
        return NULL;
    }

    solid_color_geometry_init(dest, glObject);

    dest->material = material;

    return dest;
}

void solid_color_shader_free(void) {
    glDeleteProgram(shader);
}
