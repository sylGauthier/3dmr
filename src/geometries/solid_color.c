#include "solid_color.h"
#include "shader.h"

static GLuint shader = 0;

static void draw_solid_color(const struct Geometry* geometry) {
    glUniform3fv(glGetUniformLocation(geometry->shader, "solidColor"), 1, (float*)((const struct SolidColorGeometry*)geometry)->color);
    glDrawArrays(GL_TRIANGLES, 0, geometry->glObject.numVertices);
}

void solid_color_geometry(struct SolidColorGeometry* dest, const struct GLObject* glObject, float r, float g, float b) {
    if (!shader) {
        shader = shader_compile("shaders/solid_color.vert", "shaders/solid_color.frag");
    }
    dest->geometry.shader = shader;
    dest->geometry.glObject = *glObject;
    dest->geometry.render = draw_solid_color;
    dest->color[0] = r;
    dest->color[1] = g;
    dest->color[2] = b;
}

void solid_color_shader_free(void) {
    glDeleteProgram(shader);
}
