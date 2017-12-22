#include "solid_color.h"

void draw_solid_color(const struct Geometry* geometry) {
    glUniform3fv(glGetUniformLocation(geometry->shader, "solidColor"), 1, (float*)((const struct SolidColorGeometry*)geometry)->color);
    glDrawArrays(GL_TRIANGLES, 0, geometry->glObject.numVertices);
}
