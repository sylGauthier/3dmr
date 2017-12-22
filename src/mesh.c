#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mesh.h"
#include "obj.h"

int mesh_load(struct Mesh* mesh, const char* filename, int withIndices, int withNormals, int withTexCoords) {
    FILE* objFile = NULL;
    struct OBJ obj;
    unsigned int i, j, k, l;
    int objOk = 0, ret = 0;

    if (!(objFile = fopen(filename, "r"))) {
        fprintf(stderr, "Error: could not open obj file '%s'\n", filename);
    } else if (!(objOk = obj_load(&obj, objFile))) {
        fprintf(stderr, "Error: failed to parse obj file '%s'\n", filename);
    } else if (!obj_triangulate(&obj)) {
        fprintf(stderr, "Error: failed to triangulate obj file '%s'\n", filename);
    } else {
        if (!withIndices) {
            mesh->numVertices = 3 * obj.numFaces;
            mesh->numNormals = (withNormals && obj.numNormals) ? mesh->numVertices : 0;
            mesh->numTexCoords = (withTexCoords && obj.numTexCoords) ? mesh->numTexCoords : 0;
            mesh->numIndices = 0;
            mesh->indices = NULL;
            mesh->normals = NULL;
            mesh->texCoords = NULL;
            if (!(mesh->vertices = malloc(3 * mesh->numVertices * sizeof(float)))
             || (mesh->numNormals && !(mesh->normals = malloc(3 * mesh->numVertices * sizeof(float))))
             || (mesh->numTexCoords && !(mesh->texCoords = malloc(2 * mesh->numVertices * sizeof(float))))) {
                fprintf(stderr, "Error: failed to allocate mesh buffer for obj file '%s'\n", filename);
                free(mesh->vertices);
                free(mesh->normals);
                free(mesh->texCoords);
            } else {
                for (i = k = l = 0; i < obj.numFaces; i++) {
                    for (j = 0; j < 3; j++) {
                        memcpy(mesh->vertices + k, obj.vertices + 3 * obj.faces[i].elems[j].v, 3 * sizeof(float));
                        if (withNormals && obj.numNormals) memcpy(mesh->normals + k, obj.normals + 3 * obj.faces[i].elems[j].n, 3 * sizeof(float));
                        if (withTexCoords && obj.numTexCoords) memcpy(mesh->texCoords + l, obj.texCoords + 2 * obj.faces[i].elems[j].t, 2 * sizeof(float));
                        k += 3;
                        l += 2;
                    }
                }
                ret = 1;
            }
        } else {
            mesh->numIndices = 3 * obj.numFaces;
            if (!(mesh->indices = malloc(mesh->numIndices * (1 + !!withNormals + !!withTexCoords) * sizeof(unsigned int)))) {
                fprintf(stderr, "Error: failed to allocate mesh buffers for obj file '%s'\n", filename);
            } else {
                mesh->numVertices = obj.numVertices;
                mesh->vertices = obj.vertices;
                obj.vertices = NULL;
                if (withNormals && obj.numNormals) {
                    mesh->numNormals = obj.numNormals;
                    mesh->normals = obj.normals;
                    obj.normals = NULL;
                } else {
                    mesh->numNormals = 0;
                    mesh->normals = NULL;
                }
                if (withTexCoords && obj.numTexCoords) {
                    mesh->numTexCoords = obj.numTexCoords;
                    mesh->texCoords = obj.texCoords;
                    obj.texCoords = NULL;
                } else {
                    mesh->numTexCoords = 0;
                    mesh->texCoords = NULL;
                }
                for (i = k = 0; i < obj.numFaces; i++) {
                    for (j = 0; j < 3; j++) {
                        mesh->indices[k++] = obj.faces[i].elems[j].v;
                        if (withNormals) mesh->indices[k++] = obj.faces[i].elems[j].n;
                        if (withTexCoords) mesh->indices[k++] = obj.faces[i].elems[j].t;
                    }
                }
                ret = 1;
            }
        }
    }

    if (objOk) obj_free(&obj);
    if (objFile) fclose(objFile);
    return ret;
}

void mesh_free(struct Mesh* mesh) {
    free(mesh->vertices);
    free(mesh->normals);
    free(mesh->texCoords);
    free(mesh->indices);
}

