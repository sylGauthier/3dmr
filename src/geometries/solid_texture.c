#include "solid_texture.h"
#include "shader.h"

static GLuint shader = 0;

static void draw_solid_texture(const struct Geometry* geometry) {
    glBindTexture(GL_TEXTURE_2D, ((const struct SolidTextureGeometry*)geometry)->texture);
    glDrawArrays(GL_TRIANGLES, 0, geometry->glObject.numVertices);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void solid_texture_geometry(struct SolidTextureGeometry* dest, const struct GLObject* glObject, GLuint texture) {
    if (!shader) {
        shader = shader_compile("shaders/solid_texture.vert", "shaders/solid_texture.frag");
    }
    dest->geometry.shader = shader;
    dest->geometry.glObject = *glObject;
    dest->geometry.render = draw_solid_texture;
    dest->texture = texture;
}

void solid_texture_shader_free(void) {
    glDeleteProgram(shader);
}
