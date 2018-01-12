#include "solid_texture.h"
#include "shader.h"

static GLuint shader = 0;

static void solid_texture_prerender(const struct Geometry* geometry) {
    glBindTexture(GL_TEXTURE_2D, ((const struct SolidTextureGeometry*)geometry)->texture);
}

static void solid_texture_postrender(const struct Geometry* geometry) {
    glBindTexture(GL_TEXTURE_2D, 0);
}

void solid_texture_geometry(struct SolidTextureGeometry* dest, const struct GLObject* glObject, GLuint texture) {
    dest->geometry.glObject = *glObject;
    if (!shader) {
        shader = shader_compile("shaders/solid_texture.vert", "shaders/solid_texture.frag");
    }
    dest->geometry.shader = shader;
    dest->geometry.mode = GL_FILL;
    dest->geometry.prerender = solid_texture_prerender;
    dest->geometry.postrender = solid_texture_postrender;
    dest->texture = texture;
}

void solid_texture_shader_free(void) {
    glDeleteProgram(shader);
}
