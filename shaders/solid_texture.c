#include "solid_texture.h"

void draw_solid_texture(const struct Geometry* geometry) {
    glBindTexture(GL_TEXTURE_2D, ((const struct SolidTextureGeometry*)geometry)->texture);
    glDrawArrays(GL_TRIANGLES, 0, geometry->glObject.numVertices);
    glBindTexture(GL_TEXTURE_2D, 0);
}
