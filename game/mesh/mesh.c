#include <stdlib.h>
#include <string.h>
#include "mesh.h"

int mesh_unindex(struct Mesh* mesh) {
    float *vertices = NULL, *normals = NULL, *texCoords = NULL;
    unsigned int i, index;

    if ((mesh->numVertices && !(vertices = malloc(3 * mesh->numIndices * sizeof(float))))
     || (mesh->hasNormals && !(normals = malloc(3 * mesh->numIndices * sizeof(float))))
     || (mesh->hasTexCoords && !(texCoords = malloc(2 * mesh->numIndices * sizeof(float))))) {
        free(vertices);
        free(normals);
        free(texCoords);
        return 0;
    }

    for (i = 0; i < mesh->numIndices; i++) {
        index = mesh->indices[i];
        if (vertices) memcpy(vertices + 3 * i, mesh->vertices + 3 * index, 3 * sizeof(float));
        if (normals) memcpy(normals + 3 * i, mesh->normals + 3 * index, 3 * sizeof(float));
        if (texCoords) memcpy(texCoords + 2 * i, mesh->texCoords + 2 * index, 2 * sizeof(float));
    }

    free(mesh->vertices);
    free(mesh->normals);
    free(mesh->texCoords);
    free(mesh->indices);
    mesh->vertices = vertices;
    mesh->normals = normals;
    mesh->texCoords = texCoords;
    mesh->indices = NULL;
    mesh->numVertices = mesh->numIndices;
    mesh->numIndices = 0;

    return 1;
}

unsigned int mesh_duplicate_index(struct Mesh* mesh, unsigned int index) {
    float* tmp;

    if (!(tmp = realloc(mesh->vertices, 3 * (mesh->numVertices + 1) * sizeof(float)))) {
        return 0;
    }
    mesh->vertices = tmp;
    memcpy(mesh->vertices + 3 * mesh->numVertices, mesh->vertices + 3 * index, 3 * sizeof(float));
    if (mesh->hasNormals) {
        if (!(tmp = realloc(mesh->normals, 3 * (mesh->numVertices + 1) * sizeof(float)))) {
            return 0;
        }
        mesh->normals = tmp;
        memcpy(mesh->normals + 3 * mesh->numVertices, mesh->normals + 3 * index, 3 * sizeof(float));
    }
    if (mesh->hasTexCoords) {
        if (!(tmp = realloc(mesh->texCoords, 2 * (mesh->numVertices + 1) * sizeof(float)))) {
            return 0;
        }
        mesh->texCoords = tmp;
        memcpy(mesh->texCoords + 2 * mesh->numVertices, mesh->texCoords + 2 * index, 2 * sizeof(float));
    }

    return mesh->numVertices++;
}

void mesh_free(struct Mesh* mesh) {
    free(mesh->vertices);
    free(mesh->normals);
    free(mesh->texCoords);
    free(mesh->indices);
}
