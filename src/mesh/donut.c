#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <game/math/linear_algebra.h>
#include <game/mesh/donut.h>

#define VERTEX(i) (dest->vertices + (6 * (i)))
#define NORMAL(i) (dest->vertices + (6 * (i) + 3))

int make_donut(struct Mesh* dest, float majRadius, float minRadius, unsigned int majNumPoints, unsigned int minNumPoints) {
    Vec3 c, d;
    float* n;
    double majAngle, minAngle;
    unsigned int i, j, k, v;

    dest->numVertices = minNumPoints * majNumPoints;
    dest->flags = MESH_NORMALS;
    dest->numIndices = 6 * minNumPoints * majNumPoints;

    dest->vertices = malloc(MESH_SIZEOF_VERTICES(dest));
    dest->indices = malloc(dest->numIndices * sizeof(unsigned int));
    if (!dest->vertices || !dest->indices) {
        free(dest->vertices);
        free(dest->indices);
        return 0;
    }

    c[1] = 0.0;
    v = k = 0;
    for (i = 0; i < majNumPoints; i++) {
        majAngle = 2.0 * M_PI * ((double)i) / ((double)majNumPoints);
        c[0] = majRadius * cos(majAngle);
        c[2] = majRadius * sin(majAngle);
        for (j = 0; j < minNumPoints; j++) {
            minAngle = 2.0 * M_PI * ((double)j) / ((double)minNumPoints);
            n = NORMAL(v);
            n[0] = cos(majAngle) * sin(minAngle);
            n[1] = cos(minAngle);
            n[2] = sin(majAngle) * sin(minAngle);
            mul3sv(d, minRadius, n);
            add3v(VERTEX(v), c, d);
            v++;
            dest->indices[k] = i * minNumPoints + j;
            dest->indices[k + 1] = ((i + 1) % majNumPoints) * minNumPoints + j;
            dest->indices[k + 2] = ((i + 1) % majNumPoints) * minNumPoints + ((j + 1) % minNumPoints);
            dest->indices[k + 3] = dest->indices[k];
            dest->indices[k + 4] = dest->indices[k + 2];
            dest->indices[k + 5] = i * minNumPoints + ((j + 1) % minNumPoints);
            k += 6;
        }
    }

    return 1;
}

#undef VERTEX
#undef NORMAL
#define VERTEX(i) (dest->vertices + (8 * (i)))
#define NORMAL(i) (dest->vertices + (8 * (i) + 3))
#define TEXCOORD(i) (dest->vertices + (8 * (i) + 6))

int make_donut_textured(struct Mesh* dest, float majRadius, float minRadius, unsigned int majNumPoints, unsigned int minNumPoints) {
    Vec3 c, d;
    float* n;
    double majAngle, minAngle;
    unsigned int i, j, k, v;

    dest->numVertices = (minNumPoints + 1) * (majNumPoints + 1);
    dest->flags = MESH_NORMALS | MESH_TEXCOORDS;
    dest->numIndices = 6 * minNumPoints * majNumPoints;

    dest->vertices = malloc(MESH_SIZEOF_VERTICES(dest));
    dest->indices = malloc(dest->numIndices * sizeof(unsigned int));
    if (!dest->vertices || !dest->indices) {
        free(dest->vertices);
        free(dest->indices);
        return 0;
    }

    c[1] = 0.0;
    v = 0;
    for (i = 0; i <= majNumPoints; i++) {
        majAngle = 2.0 * M_PI * ((double)i) / ((double)majNumPoints);
        c[0] = majRadius * cos(majAngle);
        c[2] = majRadius * sin(majAngle);
        for (j = 0; j <= minNumPoints; j++) {
            minAngle = 2.0 * M_PI * ((double)j) / ((double)minNumPoints);
            n = NORMAL(v);
            n[0] = cos(majAngle) * sin(minAngle);
            n[1] = cos(minAngle);
            n[2] = sin(majAngle) * sin(minAngle);
            mul3sv(d, minRadius, n);
            add3v(VERTEX(v), c, d);
            n = TEXCOORD(v);
            n[0] = ((double)i) / ((double)majNumPoints);
            n[1] = ((double)j) / ((double)minNumPoints);
            v++;
        }
    }
    k = 0;
    for (i = 0; i < majNumPoints; i++) {
        for (j = 0; j < minNumPoints; j++) {
            dest->indices[k] = i * (minNumPoints + 1) + j;
            dest->indices[k + 1] = (i + 1) * (minNumPoints + 1) + j;
            dest->indices[k + 2] = (i + 1) * (minNumPoints + 1) + (j + 1);
            dest->indices[k + 3] = dest->indices[k];
            dest->indices[k + 4] = dest->indices[k + 2];
            dest->indices[k + 5] = i * (minNumPoints + 1) + (j + 1);
            k += 6;
        }
    }

    return 1;
}
