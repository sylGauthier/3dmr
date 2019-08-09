#include <stdlib.h>
#include <math.h>
#include <game/mesh/circle.h>

#define VERTEX(i, vx, vy, vz) \
    dest->vertices[6 * (i)] = vx; \
    dest->vertices[6 * (i) + 1] = vy; \
    dest->vertices[6 * (i) + 2] = vz; \
    dest->vertices[6 * (i) + 3] = 0; \
    dest->vertices[6 * (i) + 4] = 0; \
    dest->vertices[6 * (i) + 5] = 1;

int make_circle(struct Mesh* dest, float radius, unsigned int numVertices) {
    double angle;
    unsigned int i, k;

    dest->numVertices = numVertices + 1;
    dest->flags = MESH_NORMALS;
    dest->numIndices = 3 * numVertices;

    dest->vertices = malloc(MESH_SIZEOF_VERTICES(dest));
    dest->indices = malloc(dest->numIndices * sizeof(unsigned int));
    if (!dest->vertices || !dest->indices) {
        free(dest->vertices);
        free(dest->indices);
        return 0;
    }

    k = 0;
    VERTEX(numVertices, 0, 0, 0);
    for (i = 0; i < numVertices; i++) {
        angle = 2.0 * M_PI * ((double)i) / ((double)numVertices);
        VERTEX(i, radius * cos(angle), radius * sin(angle), 0);
        dest->indices[k++] = i;
        dest->indices[k++] = (i + 1) % numVertices;
        dest->indices[k++] = numVertices;
    }

    return 1;
}
