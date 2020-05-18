#include <3dmr/mesh/mesh.h>

#ifndef TDMR_MESH_DONUT_H
#define TDMR_MESH_DONUT_H

int make_donut(struct Mesh* dest, float majRadius, float minRadius, unsigned int majNumPoints, unsigned int minNumPoints);
int make_donut_textured(struct Mesh* dest, float majRadius, float minRadius, unsigned int majNumPoints, unsigned int minNumPoints);

#endif
