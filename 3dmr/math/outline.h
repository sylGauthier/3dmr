#include <stddef.h>
#include <3dmr/math/linear_algebra.h>

#ifndef TDMR_MATH_OUTLINE_H
#define TDMR_MATH_OUTLINE_H

union OutlineSegment {
    enum OutlineSegmentType {
        OUTLINE_SEGMENT_LINEAR,
        OUTLINE_SEGMENT_QUADRATIC
    } type;
    struct OutlineSegmentLinear {
        enum OutlineSegmentType type;
        Vec2 p0;
        Vec2 p1;
    } linear;
    struct OutlineSegmentQuadratic {
        enum OutlineSegmentType type;
        Vec2 p0;
        Vec2 p1;
        Vec2 p2;
    } quadratic;
};

struct OutlineContour {
    union OutlineSegment* segments;
    size_t numSegments;
};

struct Outline {
    struct OutlineContour* contours;
    size_t numContours;
};

float distance_point_to_segment(const Vec2 p, const Vec2 p0, const Vec2 p1);
float distance_point_to_quad_bezier(const Vec2 p, const Vec2 p0, const Vec2 p1, const Vec2 p2);
float distance_point_to_outline_segment(const Vec2 p, const union OutlineSegment* s);

void outline_contour_free(struct OutlineContour* oc);
void outline_free(struct Outline* outline);

#endif
