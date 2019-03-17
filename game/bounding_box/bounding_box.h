#ifndef BOUNDING_BOX_H
#define BOUNDING_BOX_H

#include <game/math/linear_algebra.h>

struct BoundingBox {
    Vec3 center;
    Vec3 dims;
};

void bb_compute_points(const struct BoundingBox* bb, Vec3 points[]);
int bb_adapt(struct BoundingBox* bb, Vec3 point);

#endif
