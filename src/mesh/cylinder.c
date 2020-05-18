#include <3dmr/mesh/cylinder.h>
#include <3dmr/mesh/cone.h>

int make_cylinder(struct Mesh* dest, float radius, float depth, unsigned int numSidePoints) {
    return make_cone(dest, radius, radius, depth, numSidePoints);
}

int make_cylinder_textured(struct Mesh* dest, float radius, float depth, unsigned int numSidePoints) {
    return make_cone_textured(dest, radius, radius, depth, numSidePoints);
}
