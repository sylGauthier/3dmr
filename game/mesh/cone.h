#include <game/mesh/mesh.h>

#ifndef GAME_MESH_CONE_H
#define GAME_MESH_CONE_H

int make_cone(struct Mesh* dest, float radius1, float radius2, float depth, unsigned int numSidePoints);
int make_cone_textured(struct Mesh* dest, float radius1, float radius2, float depth, unsigned int numSidePoints);

#endif
