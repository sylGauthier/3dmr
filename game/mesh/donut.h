#include <game/mesh/mesh.h>

#ifndef DONUT_H
#define DONUT_H

int make_donut(struct Mesh* dest, float majRadius, float minRadius, unsigned int majNumPoints, unsigned int minNumPoints);
int make_donut_textured(struct Mesh* dest, float majRadius, float minRadius, unsigned int majNumPoints, unsigned int minNumPoints);

#endif
