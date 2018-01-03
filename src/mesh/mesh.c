#include <stdlib.h>
#include "mesh.h"

void mesh_free(struct Mesh* mesh) {
    free(mesh->vertices);
    free(mesh->normals);
    free(mesh->texCoords);
    free(mesh->indices);
}
