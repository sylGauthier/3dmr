#include <stdlib.h>
#include <string.h>
#include <3dmr/mesh/quad.h>

#define VERTEX(i, vx, vy, vz, nx, ny, nz, tu, tv) \
    dest->vertices[8 * i] = vx; \
    dest->vertices[8 * i + 1] = vy; \
    dest->vertices[8 * i + 2] = vz; \
    dest->vertices[8 * i + 3] = nx; \
    dest->vertices[8 * i + 4] = ny; \
    dest->vertices[8 * i + 5] = nz; \
    dest->vertices[8 * i + 6] = tu; \
    dest->vertices[8 * i + 7] = tv;

int make_quad(struct Mesh* dest, float width, float height) {
    float a = width / 2.0f, b = height / 2.0f;

    dest->numVertices = 6;
    dest->flags = MESH_NORMALS | MESH_TEXCOORDS;
    dest->numIndices = 0;

    dest->vertices = malloc(MESH_SIZEOF_VERTICES(dest));
    dest->indices = NULL;
    if (!dest->vertices) {
        return 0;
    }

    VERTEX(0,    -a, -b, 0,     0, 0, 1,     0, 0);
    VERTEX(1,     a, -b, 0,     0, 0, 1,     1, 0);
    VERTEX(2,     a,  b, 0,     0, 0, 1,     1, 1);
    VERTEX(3,    -a, -b, 0,     0, 0, 1,     0, 0);
    VERTEX(4,     a,  b, 0,     0, 0, 1,     1, 1);
    VERTEX(5,    -a,  b, 0,     0, 0, 1,     0, 1);

    return 1;
}
