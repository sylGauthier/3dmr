#include "solid_color.h"
#include "shader.h"

static GLuint shader = 0;

static void load_solid_color_uniforms(const struct Geometry* geometry) {
    glUniform3fv(glGetUniformLocation(geometry->shader, "solidColor"), 1, (float*)((const struct SolidColorGeometry*)geometry)->color);
}

void solid_color_geometry(struct SolidColorGeometry* dest, const struct GLObject* glObject, float r, float g, float b) {
    dest->geometry.glObject = *glObject;
    if (!shader) {
        shader = shader_compile("shaders/solid_color.vert", "shaders/solid_color.frag");
    }
    dest->geometry.shader = shader;
    dest->geometry.mode = GL_FILL;
    dest->geometry.prerender = load_solid_color_uniforms;
    dest->geometry.postrender = 0;
    dest->color[0] = r;
    dest->color[1] = g;
    dest->color[2] = b;
}

void solid_color_shader_free(void) {
    glDeleteProgram(shader);
}
