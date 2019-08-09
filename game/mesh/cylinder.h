#include <game/mesh/mesh.h>

#ifndef CYLINDER_H
#define CYLINDER_H

int make_cylinder(struct Mesh* dest, float radius, float depth, unsigned int numSidePoints);
int make_cylinder_textured(struct Mesh* dest, float radius, float depth, unsigned int numSidePoints);

#endif
