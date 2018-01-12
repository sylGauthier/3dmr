#ifndef MESH_H
#define MESH_H

struct Mesh
{
    float* vertices;
    float* normals;
    float* texCoords;
    unsigned int* indices;
    unsigned int numVertices, numNormals, numTexCoords, numIndices;
};

void mesh_free(struct Mesh*);

#endif
