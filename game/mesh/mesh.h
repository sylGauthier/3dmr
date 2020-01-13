#include <game/math/linear_algebra.h>

#ifndef MESH_H
#define MESH_H

enum MeshFlags {
    MESH_NORMALS = 1 << 0,
    MESH_TEXCOORDS = 1 << 1,
    MESH_TANGENTS = 1 << 2,
    MESH_SKIN = 1 << 3
};

struct Mesh {
    float* vertices;
    unsigned int* indices;
    unsigned int numVertices, numIndices, flags;
    struct Skin* skin;
};

#define MESH_HAS_NORMALS(mesh) ((mesh)->flags & MESH_NORMALS)
#define MESH_HAS_TEXCOORDS(mesh) ((mesh)->flags & MESH_TEXCOORDS)
#define MESH_HAS_TANGENTS(mesh) ((mesh)->flags & MESH_TANGENTS)
#define MESH_HAS_SKIN(mesh) ((mesh)->flags & MESH_SKIN)

#define MESH_FLOATS_PER_VERTEX(mesh) (3 + 3 * !!MESH_HAS_NORMALS(mesh) \
                                        + 2 * !!MESH_HAS_TEXCOORDS(mesh) \
                                        + 6 * !!MESH_HAS_TANGENTS(mesh) \
                                        + 4 * !!MESH_HAS_SKIN(mesh))
#define MESH_SIZEOF_VERTEX(mesh) (MESH_FLOATS_PER_VERTEX(mesh) * sizeof(float))
#define MESH_SIZEOF_VERTICES(mesh) (MESH_SIZEOF_VERTEX(mesh) * (mesh)->numVertices)

int mesh_unindex(struct Mesh* mesh);
unsigned int mesh_duplicate_index(struct Mesh* mesh, unsigned int index);

int mesh_compute_tangents(struct Mesh* mesh);

void mesh_free(struct Mesh*);

#endif
