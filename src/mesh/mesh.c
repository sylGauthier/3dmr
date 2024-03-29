#include <stdlib.h>
#include <string.h>
#include <3dmr/math/linear_algebra.h>
#include <3dmr/mesh/mesh.h>

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
        cur += n;
    }

    free(mesh->vertices);
    mesh->vertices = vertices;
    free(mesh->indices);
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

int mesh_compute_normals(struct Mesh* mesh) {
    Vec3 v1, v2, normal;
    unsigned int i, j, nf, nv, ind[3];

    nf = MESH_FLOATS_PER_VERTEX(mesh);
    if (mesh->numIndices) {
        nv = mesh->numIndices;
    } else {
        nv = mesh->numVertices;
    }
    for (i = 0; i < nv; i += 3) {
        if (mesh->numIndices) {
            memcpy(ind, mesh->indices + i, sizeof(ind));
        } else {
            ind[0] = i;
            ind[1] = i + 1;
            ind[2] = i + 2;
        }
        sub3v(v1, mesh->vertices + nf * ind[1], mesh->vertices + nf * ind[0]);
        sub3v(v2, mesh->vertices + nf * ind[2], mesh->vertices + nf * ind[0]);
        cross3(normal, v1, v2);
        normalize3(normal);
        for (j = 0; j < 3; j++) {
            memcpy(mesh->vertices + nf * ind[j] + 3, normal, sizeof(Vec3));
        }
    }
    return 1;
}

int mesh_add_normals(struct Mesh* mesh) {
    float *tmp;
    unsigned int i, n, m;

    if (MESH_HAS_NORMALS(mesh)) {
        return 1;
    }
    m = MESH_FLOATS_PER_VERTEX(mesh);
    mesh->flags |= MESH_NORMALS;
    n = MESH_FLOATS_PER_VERTEX(mesh);
    if (!(tmp = malloc(n * mesh->numVertices * sizeof(float)))) {
        mesh->flags &= ~MESH_NORMALS;
        return 0;
    }
    for (i = 0; i < mesh->numVertices; i++) {
        memcpy(tmp + n * i, mesh->vertices + m * i, m * sizeof(float));
    }
    free(mesh->vertices);
    mesh->vertices = tmp;
    return mesh_compute_normals(mesh);
}

int mesh_compute_tangents(struct Mesh* mesh) {
    Mat2 dTC;
    Vec3 d1, d2, T, B;
    float oneOverDet;
    unsigned int i, j, ind[3], n, numVertices;

    if (!MESH_HAS_TANGENTS(mesh) || !MESH_HAS_NORMALS(mesh) || !MESH_HAS_TEXCOORDS(mesh)) {
        return 0;
    }
    n = MESH_FLOATS_PER_VERTEX(mesh);
    if (mesh->numIndices) {
        numVertices = mesh->numIndices;
    } else {
        numVertices = mesh->numVertices;
    }
    for (i = 0; i < numVertices; i += 3) {
        if (mesh->numIndices) {
            ind[0] = mesh->indices[i];
            ind[1] = mesh->indices[i + 1];
            ind[2] = mesh->indices[i + 2];
        } else {
            ind[0] = i;
            ind[1] = i + 1;
            ind[2] = i + 2;
        }
        sub2v(dTC[0], mesh->vertices + n * ind[1] + 6, mesh->vertices + n * ind[0] + 6);
        sub2v(dTC[1], mesh->vertices + n * ind[2] + 6, mesh->vertices + n * ind[0] + 6);
        sub3v(d1, mesh->vertices + n * ind[1], mesh->vertices + n * ind[0]);
        sub3v(d2, mesh->vertices + n * ind[2], mesh->vertices + n * ind[0]);
        oneOverDet = 1.0f / det2(MAT_CONST_CAST(dTC));
        T[0] = oneOverDet * (d1[0] * dTC[1][1] - d2[0] * dTC[0][1]);
        T[1] = oneOverDet * (d1[1] * dTC[1][1] - d2[1] * dTC[0][1]);
        T[2] = oneOverDet * (d1[2] * dTC[1][1] - d2[2] * dTC[0][1]);
        B[0] = oneOverDet * (d1[0] * dTC[1][0] - d2[0] * dTC[0][0]);
        B[1] = oneOverDet * (d1[1] * dTC[1][0] - d2[1] * dTC[0][0]);
        B[2] = oneOverDet * (d1[2] * dTC[1][0] - d2[2] * dTC[0][0]);
        for (j = 0; j < 3; j++) {
            mul3sv(d1, dot3(mesh->vertices + n * ind[j] + 3, T), mesh->vertices + n * ind[j] + 3); /* (N dot T) N */
            sub3v(mesh->vertices + n * ind[j] + 8, T, d1); /* T' <- T - (N dot T) N */
            mul3sv(d1, dot3(mesh->vertices + n * ind[j] + 3, B), mesh->vertices + n * ind[j] + 3); /* (N dot B) N */
            sub3v(mesh->vertices + n * ind[j] + 11, B, d1); /* B' <- B - (N dot B) N */
            mul3sv(d1, dot3(mesh->vertices + n * ind[j] + 8, B) / norm3sq(mesh->vertices + n * ind[j] + 8), mesh->vertices + n * ind[j] + 8); /* (T' dot B) T' / (T')² */
            decr3v(mesh->vertices + n * ind[j] + 11, d1); /* B' <- B' - (T' dot B) T' / (T')² */
            normalize3(mesh->vertices + n * ind[j] + 8);
            normalize3(mesh->vertices + n * ind[j] + 11);
        }
    }
    return 1;
}

int mesh_add_tangents(struct Mesh* mesh) {
    float *tmp;
    unsigned int i, n, m;

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
    return mesh_compute_tangents(mesh);
}

void mesh_translate(struct Mesh* mesh, Vec3 t) {
    unsigned int i;

    for (i = 0; i < mesh->numVertices; i++) {
        incr3v(mesh->vertices + i * MESH_FLOATS_PER_VERTEX(mesh), t);
    }
}

void mesh_rotate(struct Mesh* mesh, Vec3 axis, float angle) {
    unsigned int i;
    Mat3 rot, invNormal;

    load_rot3(rot, axis, angle);
    if (MESH_HAS_NORMALS(mesh)) {
        invert3m(invNormal, MAT_CONST_CAST(rot));
        transpose3m(invNormal);
    }

    for (i = 0; i < mesh->numVertices; i++) {
        Vec3 tmp;

        memcpy(tmp, mesh->vertices + i * MESH_FLOATS_PER_VERTEX(mesh), sizeof(Vec3));
        mul3mv(mesh->vertices + i * MESH_FLOATS_PER_VERTEX(mesh), MAT_CONST_CAST(rot), tmp);
        if (MESH_HAS_NORMALS(mesh)) {
            memcpy(tmp, mesh->vertices + i * MESH_FLOATS_PER_VERTEX(mesh) + 3, sizeof(Vec3));
            mul3mv(mesh->vertices + i * MESH_FLOATS_PER_VERTEX(mesh) + 3, MAT_CONST_CAST(invNormal), tmp);
        }
        if (MESH_HAS_TANGENTS(mesh)) {
            memcpy(tmp, mesh->vertices + i * MESH_FLOATS_PER_VERTEX(mesh) + 8, sizeof(Vec3));
            mul3mv(mesh->vertices + i * MESH_FLOATS_PER_VERTEX(mesh) + 8, MAT_CONST_CAST(invNormal), tmp);
            memcpy(tmp, mesh->vertices + i * MESH_FLOATS_PER_VERTEX(mesh) + 11, sizeof(Vec3));
            mul3mv(mesh->vertices + i * MESH_FLOATS_PER_VERTEX(mesh) + 11, MAT_CONST_CAST(invNormal), tmp);
        }
    }
}

void mesh_scale(struct Mesh* mesh, Vec3 s) {
    unsigned int i, j;

    for (i = 0; i < mesh->numVertices; i++) {
        for (j = 0; j < 3; j++) {
            mesh->vertices[i * MESH_FLOATS_PER_VERTEX(mesh) + j] *= s[j];
        }
    }
}

void mesh_free(struct Mesh* mesh) {
    free(mesh->vertices);
    free(mesh->indices);
}
