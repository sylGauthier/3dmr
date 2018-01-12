#ifndef BOUNDING_BOX_H
#define BOUNDING_BOX_H

struct ABoundingBox {
    Vec3 center;

    /*width: X-axis, height: Y-axis, depth: Z-axis*/
    float width,height,depth;
};

#endif
