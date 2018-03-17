#include "mesh.h"

#ifndef OBJ_H
#define OBJ_H

struct OBJ {
    float *vertices, *normals, *texCoords;
    struct OBJFace {
        struct OBJFaceElem {
            unsigned int v, t, n;
        } *elems;
        unsigned int numElems;
    } *faces;
    unsigned int numVertices, numNormals, numTexCoords, numFaces;
};

int obj_load(struct OBJ* obj, FILE* file);
void obj_free(struct OBJ* obj);

int obj_triangulate(struct OBJ* obj);

int make_obj(struct Mesh* dest, const char* filename, int withIndices, int withNormals, int withTexCoords);

int mesh_save_obj(const struct Mesh* mesh, FILE* dest);

#endif
