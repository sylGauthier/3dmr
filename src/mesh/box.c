#include <stdlib.h>
#include <string.h>
#include <3dmr/mesh/box.h>

#define VERTEX(i, vx, vy, vz, nx, ny, nz, tu, tv) \
    dest->vertices[8 * i] = vx; \
    dest->vertices[8 * i + 1] = vy; \
    dest->vertices[8 * i + 2] = vz; \
    dest->vertices[8 * i + 3] = nx; \
    dest->vertices[8 * i + 4] = ny; \
    dest->vertices[8 * i + 5] = nz; \
    dest->vertices[8 * i + 6] = tu; \
    dest->vertices[8 * i + 7] = tv;

int make_box(struct Mesh* dest, float width, float height, float depth) {
    float a = width / 2.0f, b = height / 2.0f, c = depth / 2.0f;

    dest->numVertices = 36;
    dest->flags = MESH_NORMALS | MESH_TEXCOORDS;
    dest->numIndices = 0;

    dest->vertices = malloc(MESH_SIZEOF_VERTICES(dest));
    dest->indices = NULL;
    if (!dest->vertices) {
        return 0;
    }

    VERTEX( 0,    -a, -b, -c,     0,  0, -1,     0, 0);
    VERTEX( 1,     a,  b, -c,     0,  0, -1,     1, 1);
    VERTEX( 2,     a, -b, -c,     0,  0, -1,     1, 0);
    VERTEX( 3,    -a, -b, -c,     0,  0, -1,     0, 0);
    VERTEX( 4,    -a,  b, -c,     0,  0, -1,     0, 1);
    VERTEX( 5,     a,  b, -c,     0,  0, -1,     1, 1);
    VERTEX( 6,    -a, -b,  c,     0,  0,  1,     0, 0);
    VERTEX( 7,     a, -b,  c,     0,  0,  1,     1, 0);
    VERTEX( 8,     a,  b,  c,     0,  0,  1,     1, 1);
    VERTEX( 9,    -a, -b,  c,     0,  0,  1,     0, 0);
    VERTEX(10,     a,  b,  c,     0,  0,  1,     1, 1);
    VERTEX(11,    -a,  b,  c,     0,  0,  1,     0, 1);
    VERTEX(12,    -a, -b, -c,    -1,  0,  0,     0, 0);
    VERTEX(13,    -a,  b,  c,    -1,  0,  0,     1, 1);
    VERTEX(14,    -a,  b, -c,    -1,  0,  0,     1, 0);
    VERTEX(15,    -a, -b, -c,    -1,  0,  0,     0, 0);
    VERTEX(16,    -a, -b,  c,    -1,  0,  0,     0, 1);
    VERTEX(17,    -a,  b,  c,    -1,  0,  0,     1, 1);
    VERTEX(18,     a, -b, -c,     1,  0,  0,     0, 0);
    VERTEX(19,     a,  b, -c,     1,  0,  0,     1, 0);
    VERTEX(20,     a,  b,  c,     1,  0,  0,     1, 1);
    VERTEX(21,     a, -b, -c,     1,  0,  0,     0, 0);
    VERTEX(22,     a,  b,  c,     1,  0,  0,     1, 1);
    VERTEX(23,     a, -b,  c,     1,  0,  0,     0, 1);
    VERTEX(24,    -a, -b, -c,     0, -1,  0,     0, 0);
    VERTEX(25,     a, -b, -c,     0, -1,  0,     1, 0);
    VERTEX(26,     a, -b,  c,     0, -1,  0,     1, 1);
    VERTEX(27,    -a, -b, -c,     0, -1,  0,     0, 0);
    VERTEX(28,     a, -b,  c,     0, -1,  0,     1, 1);
    VERTEX(29,    -a, -b,  c,     0, -1,  0,     0, 1);
    VERTEX(30,    -a,  b, -c,     0,  1,  0,     0, 0);
    VERTEX(31,     a,  b,  c,     0,  1,  0,     1, 1);
    VERTEX(32,     a,  b, -c,     0,  1,  0,     1, 0);
    VERTEX(33,    -a,  b, -c,     0,  1,  0,     0, 0);
    VERTEX(34,    -a,  b,  c,     0,  1,  0,     0, 1);
    VERTEX(35,     a,  b,  c,     0,  1,  0,     1, 1);

    return 1;
}
