#include <3dmr/math/linear_algebra.h>

#ifndef TDMR_MESH_H
#define TDMR_MESH_H

enum MeshFlags {
    MESH_NORMALS = 1 << 0,
    MESH_TEXCOORDS = 1 << 1,
    MESH_TANGENTS = 1 << 2,
    MESH_SKIN = 1 << 3
};

struct Mesh {
    float* vertices;
    unsigned int* indices;
    unsigned int numVertices, numIndices;
    enum MeshFlags flags;
};

#define MESH_HAS_NORMALS(mesh) ((mesh)->flags & MESH_NORMALS)
#define MESH_HAS_TEXCOORDS(mesh) ((mesh)->flags & MESH_TEXCOORDS)
#define MESH_HAS_TANGENTS(mesh) ((mesh)->flags & MESH_TANGENTS)
#define MESH_HAS_SKIN(mesh) ((mesh)->flags & MESH_SKIN)

#define MESH_FLOATS_PER_VERTEX(mesh) (3 + 3 * !!MESH_HAS_NORMALS(mesh) \
                                        + 2 * !!MESH_HAS_TEXCOORDS(mesh) \
                                        + 4 * !!MESH_HAS_TANGENTS(mesh) \
                                        + 4 * !!MESH_HAS_SKIN(mesh))
#define MESH_SIZEOF_VERTEX(mesh) (MESH_FLOATS_PER_VERTEX(mesh) * sizeof(float))
#define MESH_SIZEOF_VERTICES(mesh) (MESH_SIZEOF_VERTEX(mesh) * (mesh)->numVertices)

int mesh_unindex(struct Mesh* mesh);
unsigned int mesh_duplicate_index(struct Mesh* mesh, unsigned int index);

/* compute already existing normals in-place */
int mesh_compute_normals(struct Mesh* mesh);

/* copy/extend vertex array to add normals then compute them */
int mesh_add_normals(struct Mesh* mesh);

/* compute already existing tangents in-place */
int mesh_compute_tangents(struct Mesh* mesh);

/* copy/extend vertex array to add tangents then compute them */
int mesh_add_tangents(struct Mesh* mesh);

void mesh_translate(struct Mesh* mesh, Vec3 t);
void mesh_rotate(struct Mesh* mesh, Vec3 axis, float angle);
void mesh_scale(struct Mesh* mesh, Vec3 scale);

void mesh_free(struct Mesh*);

#endif
