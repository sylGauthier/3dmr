#include <3dmr/mesh/mesh.h>

#ifndef TDMR_MESH_CYLINDER_H
#define TDMR_MESH_CYLINDER_H

int make_cylinder(struct Mesh* dest, float radius, float depth, unsigned int numSidePoints);
int make_cylinder_textured(struct Mesh* dest, float radius, float depth, unsigned int numSidePoints);

#endif
