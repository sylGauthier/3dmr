#include <GL/glew.h>
#include "material.h"

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

int mesh_load(struct Mesh* mesh, const char* filename, int withIndices, int withNormals, int withTexCoords);
void mesh_free(struct Mesh*);

#endif
