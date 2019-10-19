#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <game/math/linear_algebra.h>
#include <game/mesh/uvsphere.h>

#define VERTEX(i) (dest->vertices + (6 * (i)))
#define NORMAL(i) (dest->vertices + (6 * (i) + 3))

int make_uvsphere(struct Mesh* dest, float radius, unsigned int longNumPoints, unsigned int latNumPoints) {
    float* n;
    double latAngle, longAngle, y;
    unsigned int i, j, k, v;

    dest->numVertices = longNumPoints * latNumPoints;
    dest->flags = MESH_NORMALS;
    dest->numIndices = 6 * longNumPoints * latNumPoints;

    dest->vertices = malloc(MESH_SIZEOF_VERTICES(dest));
    dest->indices = malloc(dest->numIndices * sizeof(unsigned int));
    if (!dest->vertices || !dest->indices) {
        free(dest->vertices);
        free(dest->indices);
        return 0;
    }

    v = k = 0;
    for (i = 0; i < latNumPoints; i++) {
        latAngle = M_PI * (((double)i) / ((double)latNumPoints) - 0.5);
        y = sin(latAngle);
        for (j = 0; j < longNumPoints; j++) {
            longAngle = 2.0 * M_PI * ((double)j) / ((double)longNumPoints);
            n = NORMAL(v);
            n[0] = cos(longAngle) * cos(latAngle);
            n[1] = y;
            n[2] = sin(longAngle) * cos(latAngle);
            mul3sv(VERTEX(v), radius, n);
            v++;
            dest->indices[k] = i * longNumPoints + j;
            dest->indices[k + 1] = ((i + 1) % latNumPoints) * longNumPoints + j;
            dest->indices[k + 2] = ((i + 1) % latNumPoints) * longNumPoints + ((j + 1) % longNumPoints);
            dest->indices[k + 3] = dest->indices[k];
            dest->indices[k + 4] = dest->indices[k + 2];
            dest->indices[k + 5] = i * longNumPoints + ((j + 1) % longNumPoints);
            k += 6;
        }
    }

    return 1;
}
