#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <3dmr/mesh/icosphere.h>

static const unsigned int initialIndices[] = {
    0, 11, 5, 0, 5, 1, 0, 1, 7, 0, 7, 10, 0, 10, 11,
    1, 5, 9, 5, 11, 4, 11, 10, 2, 10, 7, 6, 7, 1, 8,
    3, 9, 4, 3, 4, 2, 3, 2, 6, 3, 6, 8, 3, 8, 9,
    4, 9, 5, 2, 4, 11, 6, 2, 10, 8, 6, 7, 9, 8, 1
};

static unsigned int middle_point(unsigned int v1, unsigned int v2, float radius, float* vertices, unsigned int* numVertices, unsigned int* map) {
    unsigned int i, *mapCur;

    if (v1 > v2) {
        unsigned int tmp = v1;
        v1 = v2;
        v2 = tmp;
    }
    mapCur = map + v1 * 12;
    for (i = 0; i < 6 && *mapCur != UINT_MAX && *mapCur != v2; i++) {
        mapCur += 2;
    }
    if (i == 6) {
        return UINT_MAX;
    } else if (*mapCur != v2) {
        float x = (vertices[6 * v1    ] + vertices[6 * v2    ]) / 2.0f;
        float y = (vertices[6 * v1 + 1] + vertices[6 * v2 + 1]) / 2.0f;
        float z = (vertices[6 * v1 + 2] + vertices[6 * v2 + 2]) / 2.0f;
        float factor = radius / sqrt(x * x + y * y + z * z);
        vertices += 6 * (*numVertices);
        vertices[0] = x * factor;
        vertices[1] = y * factor;
        vertices[2] = z * factor;
        *mapCur++ = v2;
        *mapCur = (*numVertices)++;
    } else {
        mapCur++;
    }
    return *mapCur;
}

int make_icosphere(struct Mesh* dest, float radius, unsigned int numSplits) {
    float A = radius * 0.52573111211913f;
    float B = radius * 0.85065080835204f;
    float factor;
    unsigned int i, a, b, c, v1, v2, v3, q = 1 << (numSplits << 1), numVertices, numFaces, *indicesEnd, *map;

    /* Allocate vertices, normals, indices arrays */
    dest->numVertices = 12 + 10 * (q - 1);
    dest->flags = MESH_NORMALS;
    dest->numIndices = 60 * q;
    dest->vertices = malloc(dest->numVertices * 6 * sizeof(float));
    dest->indices = malloc(dest->numIndices * sizeof(unsigned int));
    map = malloc(dest->numVertices * 12 * sizeof(unsigned int));
    if (!dest->vertices || !dest->indices || !map) {
        free(dest->vertices);
        free(dest->indices);
        free(map);
        return 0;
    }

    /* Initialization */
    numVertices = 12;
    numFaces = 20;

    dest->vertices[ 0] = -A; dest->vertices[ 1] =  B; dest->vertices[ 2] = 0.0f;
    dest->vertices[ 6] =  A; dest->vertices[ 7] =  B; dest->vertices[ 8] = 0.0f;
    dest->vertices[12] = -A; dest->vertices[13] = -B; dest->vertices[14] = 0.0f;
    dest->vertices[18] =  A; dest->vertices[19] = -B; dest->vertices[20] = 0.0f;

    dest->vertices[24] = 0.0f; dest->vertices[25] = -A; dest->vertices[26] =  B;
    dest->vertices[30] = 0.0f; dest->vertices[31] =  A; dest->vertices[32] =  B;
    dest->vertices[36] = 0.0f; dest->vertices[37] = -A; dest->vertices[38] = -B;
    dest->vertices[42] = 0.0f; dest->vertices[43] =  A; dest->vertices[44] = -B;

    dest->vertices[48] =  B; dest->vertices[49] = 0.0f; dest->vertices[50] = -A;
    dest->vertices[54] =  B; dest->vertices[55] = 0.0f; dest->vertices[56] =  A;
    dest->vertices[60] = -B; dest->vertices[61] = 0.0f; dest->vertices[62] = -A;
    dest->vertices[66] = -B; dest->vertices[67] = 0.0f; dest->vertices[68] =  A;

    memcpy(dest->indices, initialIndices, sizeof(initialIndices));
    indicesEnd = dest->indices + 60;

    /* Splits */
    while (numSplits--) {
        for (i = 0; i < 12 * numVertices; i++) {
            map[i] = UINT_MAX;
        }
        for (i = 0; i < numFaces; i++) {
            v1 = dest->indices[3 * i];
            v2 = dest->indices[3 * i + 1];
            v3 = dest->indices[3 * i + 2];
            a = middle_point(v1, v2, radius, dest->vertices, &numVertices, map);
            b = middle_point(v2, v3, radius, dest->vertices, &numVertices, map);
            c = middle_point(v1, v3, radius, dest->vertices, &numVertices, map);
            dest->indices[3 * i] = a;
            dest->indices[3 * i + 1] = b;
            dest->indices[3 * i + 2] = c;
            indicesEnd[0] = v1; indicesEnd[1] = a; indicesEnd[2] = c;
            indicesEnd[3] = v2; indicesEnd[4] = b; indicesEnd[5] = a;
            indicesEnd[6] = v3; indicesEnd[7] = c; indicesEnd[8] = b;
            indicesEnd += 9;
        }
        numFaces *= 4;
    }

    /* Normals */
    for (q = i = 0; i < numVertices; i++) {
        factor = 1.0f / sqrt(dest->vertices[q] * dest->vertices[q] + dest->vertices[q + 1] * dest->vertices[q + 1] + dest->vertices[q + 2] * dest->vertices[q + 2]);
        dest->vertices[q + 3] = dest->vertices[q] * factor;
        dest->vertices[q + 4] = dest->vertices[q + 1] * factor;
        dest->vertices[q + 5] = dest->vertices[q + 2] * factor;
        q += 6;
    }

    free(map);
    return 1;
}
