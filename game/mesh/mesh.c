#include <stdlib.h>
#include <string.h>
#include <game/math/linear_algebra.h>
#include "mesh.h"

int mesh_unindex(struct Mesh* mesh) {
    float *vertices, *cur;
    unsigned int i, index, n = MESH_FLOATS_PER_VERTEX(mesh);

    if (!mesh->numIndices) {
        return 1;
    }

    if (!(vertices = malloc(n * sizeof(float) * mesh->numIndices))) {
        return 0;
    }

    cur = vertices;
    for (i = 0; i < mesh->numIndices; i++) {
        index = mesh->indices[i];
        memcpy(cur, mesh->vertices + n * index, n * sizeof(float));
    }

    free(mesh->vertices);
    mesh->vertices = vertices;
    mesh->indices = NULL;
    mesh->numVertices = mesh->numIndices;
    mesh->numIndices = 0;

    return 1;
}

unsigned int mesh_duplicate_index(struct Mesh* mesh, unsigned int index) {
    float* tmp;
    unsigned int n = MESH_FLOATS_PER_VERTEX(mesh);

    if (!(tmp = realloc(mesh->vertices, n * (mesh->numVertices + 1) * sizeof(float)))) {
        return 0;
    }
    mesh->vertices = tmp;
    memcpy(mesh->vertices + n * mesh->numVertices, mesh->vertices + n * index, n * sizeof(float));

    return mesh->numVertices++;
}

int mesh_compute_tangents(struct Mesh* mesh) {
    Mat2 dTC, inv;
    Vec3 d1, d2;
    float *tmp;
    unsigned int i, i0, i1, i2, n, m, numVertices;

    if (MESH_HAS_TANGENTS(mesh)) {
        return 1;
    } else if (!MESH_HAS_NORMALS(mesh) || !MESH_HAS_TEXCOORDS(mesh)) {
        return 0;
    }
    m = MESH_FLOATS_PER_VERTEX(mesh);
    mesh->flags |= MESH_TANGENTS;
    n = MESH_FLOATS_PER_VERTEX(mesh);
    if (!(tmp = malloc(n * mesh->numVertices * sizeof(float)))) {
        mesh->flags &= ~MESH_TANGENTS;
        return 0;
    }
    for (i = 0; i < mesh->numVertices; i++) {
        memcpy(tmp + n * i, mesh->vertices + m * i, m * sizeof(float));
    }
    free(mesh->vertices);
    mesh->vertices = tmp;

    if (mesh->numIndices) {
        numVertices = mesh->numIndices;
    } else {
        numVertices = mesh->numVertices;
    }
    for (i = 0; i < numVertices; i += 3) {
        if (mesh->numIndices) {
            i0 = mesh->indices[i];
            i1 = mesh->indices[i + 1];
            i2 = mesh->indices[i + 2];
        } else {
            i0 = i;
            i1 = i + 1;
            i2 = i + 2;
        }
        sub2v(dTC[0], mesh->vertices + n * i1 + 6, mesh->vertices + n * i0 + 6);
        sub2v(dTC[1], mesh->vertices + n * i2 + 6, mesh->vertices + n * i0 + 6);
        invert2m(inv, dTC);
        sub3v(d1, mesh->vertices + n * i1, mesh->vertices + n * i0);
        sub3v(d2, mesh->vertices + n * i2, mesh->vertices + n * i0);
        mesh->vertices[n * i0 +  8 + 0] = d1[0] * inv[0][0] + d2[0] * inv[0][1];
        mesh->vertices[n * i0 +  8 + 1] = d1[1] * inv[0][0] + d2[1] * inv[0][1];
        mesh->vertices[n * i0 +  8 + 2] = d1[2] * inv[0][0] + d2[2] * inv[0][1];
        mesh->vertices[n * i0 + 11 + 0] = d1[0] * inv[1][0] + d2[0] * inv[1][1];
        mesh->vertices[n * i0 + 11 + 1] = d1[1] * inv[1][0] + d2[1] * inv[1][1];
        mesh->vertices[n * i0 + 11 + 2] = d1[2] * inv[1][0] + d2[2] * inv[1][1];
        mul3sv(d1, dot3(mesh->vertices + n * i0 + 3, mesh->vertices + n * i0 + 8), mesh->vertices + n * i0 + 3); /* (N dot T) N */
        decr3v(mesh->vertices + n * i0 + 8, d1); /* T <- T - (N dot T) N */
        normalize3(mesh->vertices + n * i0 + 8);
        cross3(d1, mesh->vertices + n * i0 + 3, mesh->vertices + n * i0 + 8);
        if (dot3(d1, mesh->vertices + n * i0 + 11) < 0.0f) {
            neg3v(d1);
        }
        memcpy(mesh->vertices + n * i0 + 11, d1, sizeof(d1));
        normalize3(mesh->vertices + n * i0 + 11);
        memcpy(mesh->vertices + n * i1 + 8, mesh->vertices + n * i0 + 8, 6 * sizeof(float));
        memcpy(mesh->vertices + n * i2 + 8, mesh->vertices + n * i0 + 8, 6 * sizeof(float));
    }
    return 1;
}

void mesh_free(struct Mesh* mesh) {
    free(mesh->vertices);
    free(mesh->indices);
}
