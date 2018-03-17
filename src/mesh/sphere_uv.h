#include "mesh/mesh.h"

#ifndef SPHERE_UV_H
#define SPHERE_UV_H

enum SphereMapType {
    SPHERE_MAP_CENTRAL_CYLINDRICAL, /* https://en.wikipedia.org/wiki/Central_cylindrical_projection */
    SPHERE_MAP_MERCATOR, /* https://en.wikipedia.org/wiki/Mercator_projection */
    SPHERE_MAP_MILLER, /* https://en.wikipedia.org/wiki/Miller_cylindrical_projection */
    SPHERE_MAP_EQUIRECTANGULAR, /* https://en.wikipedia.org/wiki/Equirectangular_projection */
    NUM_SPHERE_MAP
};

/* texWidth and texHeight are the fraction of the total texture size that contains the map (between 0.0 and 1.0) */
/* texRatio is the ratio width/height of the texture total size (1.0 if the texture is square) */
int compute_sphere_uv(struct Mesh* sphere, double texWidth, double texHeight, double texRatio, enum SphereMapType type);

#endif
