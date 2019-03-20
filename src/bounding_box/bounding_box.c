#include <stdio.h>

#include <game/bounding_box/bounding_box.h>

void bb_compute_points(const struct BoundingBox* bb, Vec3 points[]) {
    points[0][0] = bb->center[0] + bb->dims[0]/2.;
    points[0][1] = bb->center[1] + bb->dims[1]/2.;
    points[0][2] = bb->center[2] + bb->dims[2]/2.;

    points[1][0] = bb->center[0] - bb->dims[0]/2.;
    points[1][1] = bb->center[1] + bb->dims[1]/2.;
    points[1][2] = bb->center[2] + bb->dims[2]/2.;

    points[2][0] = bb->center[0] - bb->dims[0]/2.;
    points[2][1] = bb->center[1] + bb->dims[1]/2.;
    points[2][2] = bb->center[2] - bb->dims[2]/2.;

    points[3][0] = bb->center[0] + bb->dims[0]/2.;
    points[3][1] = bb->center[1] + bb->dims[1]/2.;
    points[3][2] = bb->center[2] - bb->dims[2]/2.;

    points[4][0] = bb->center[0] + bb->dims[0]/2.;
    points[4][1] = bb->center[1] - bb->dims[1]/2.;
    points[4][2] = bb->center[2] + bb->dims[2]/2.;

    points[5][0] = bb->center[0] - bb->dims[0]/2.;
    points[5][1] = bb->center[1] - bb->dims[1]/2.;
    points[5][2] = bb->center[2] + bb->dims[2]/2.;

    points[6][0] = bb->center[0] - bb->dims[0]/2.;
    points[6][1] = bb->center[1] - bb->dims[1]/2.;
    points[6][2] = bb->center[2] - bb->dims[2]/2.;

    points[7][0] = bb->center[0] + bb->dims[0]/2.;
    points[7][1] = bb->center[1] - bb->dims[1]/2.;
    points[7][2] = bb->center[2] - bb->dims[2]/2.;
}

int bb_adapt(struct BoundingBox* bb, Vec3 point) {
    int changed = 0;
    Vec3 min, max;

    min[0] = bb->center[0] - bb->dims[0]/2.;
    min[1] = bb->center[1] - bb->dims[1]/2.;
    min[2] = bb->center[2] - bb->dims[2]/2.;
    max[0] = bb->center[0] + bb->dims[0]/2.;
    max[1] = bb->center[1] + bb->dims[1]/2.;
    max[2] = bb->center[2] + bb->dims[2]/2.;
    if (point[0] < min[0]) {
        min[0] = point[0];
        changed = 1;
    }
    if (point[1] < min[1]) {
        min[1] = point[1];
        changed = 1;
    }
    if (point[2] < min[2]) {
        min[2] = point[2];
        changed = 1;
    }
    if (point[0] > max[0]) {
        max[0] = point[0];
        changed = 1;
    }
    if (point[1] > max[1]) {
        max[1] = point[1];
        changed = 1;
    }
    if (point[2] > max[2]) {
        max[2] = point[2];
        changed = 1;
    }
    if (changed) {
        bb->center[0] = (min[0] + max[0])/2.;
        bb->center[1] = (min[1] + max[1])/2.;
        bb->center[2] = (min[2] + max[2])/2.;
        bb->dims[0] = max[0]-min[0];
        bb->dims[1] = max[1]-min[1];
        bb->dims[2] = max[2]-min[2];
    }
    return changed;
}
