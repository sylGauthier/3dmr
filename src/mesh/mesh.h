#ifndef MESH_H
#define MESH_H

struct Mesh {
    float* vertices;
    float* normals;
    float* texCoords;
    unsigned int* indices;
    unsigned int numVertices, numNormals, numTexCoords, numIndices;
};

int mesh_unindex(struct Mesh* mesh);

void mesh_free(struct Mesh*);

#endif
