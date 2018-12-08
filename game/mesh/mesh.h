#ifndef MESH_H
#define MESH_H

struct Mesh {
    float* vertices;
    float* normals;
    float* texCoords;
    unsigned int* indices;
    unsigned int numVertices, numIndices;
    int hasNormals, hasTexCoords;
};

int mesh_unindex(struct Mesh* mesh);
unsigned int mesh_duplicate_index(struct Mesh* mesh, unsigned int index);

void mesh_free(struct Mesh*);

#endif
