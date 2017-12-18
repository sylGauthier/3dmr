#include <GL/glew.h>
#include "material.h"

#ifndef MESH_H
#define MESH_H

struct Mesh
{
    struct Material mat;
    unsigned int numVertices;
    float* vertices;
    float* normals;
    float* texCoords;
};

void mesh_load(struct Mesh* mesh, const char* modeldir, const char* filename);
void mesh_free(struct Mesh*);

#endif
