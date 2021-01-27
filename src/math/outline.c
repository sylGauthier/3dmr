#include <stdlib.h>
#include <math.h>
#include <3dmr/math/outline.h>
#include <3dmr/math/interp.h>
#include <3dmr/math/polynom.h>

float distance_point_to_segment(const Vec2 p, const Vec2 p0, const Vec2 p1) {
    Vec2 p01, p0p;
    float t;

    sub2v(p01, p1, p0);
    sub2v(p0p, p, p0);
    t = clamp(dot2(p0p, p01) / dot2(p01, p01), 0.0f, 1.0f);
    mul2sv(p0p, t, p01);
    incr2v(p0p, p0);
    decr2v(p0p, p);
    return norm2(p0p);
}

float distance_point_to_quad_bezier(const Vec2 p, const Vec2 p0, const Vec2 p1, const Vec2 p2) {
    Vec2 p01, p012, p0p, p2p, d, d1, d2;
    float t[3], coefs[4];
    float ti, dist, mindist;
    int i, numRoots;

    sub2v(p01, p1, p0);
    sub2v(p012, p2, p1);
    decr2v(p012, p01);
    sub2v(p0p, p, p0);
    sub2v(p2p, p, p2);
    coefs[3] = dot2(p012, p012);
    coefs[2] = 3.0f * dot2(p01, p012);
    coefs[1] = 2.0f * dot2(p01, p01) - dot2(p012, p0p);
    coefs[0] = -dot2(p01, p0p);
    mindist = norm2(p0p);
    if ((dist = norm2(p2p)) < mindist) {
        mindist = dist;
    }
    numRoots = polynom_roots_3(t, coefs);
    for (i = 0; i < numRoots; i++) {
        ti = t[i];
        if (ti > 0.0f && ti < 1.0f) {
            mul2sv(d1, -(2.0f * ti), p01);
            mul2sv(d2, -(ti * ti), p012);
            add2v(d, d1, d2);
            incr2v(d, p0p);
            if ((dist = norm2(d)) < mindist) {
                mindist = dist;
            }
        }
    }
    return mindist;
}

float distance_point_to_outline_segment(const Vec2 p, const union OutlineSegment* s) {
    switch (s->type) {
        case OUTLINE_SEGMENT_LINEAR:    return distance_point_to_segment(p, s->linear.p0, s->linear.p1);
        case OUTLINE_SEGMENT_QUADRATIC: return distance_point_to_quad_bezier(p, s->quadratic.p0, s->quadratic.p1, s->quadratic.p2);
    }
    return acos(2); /* not a number */
}

void outline_contour_free(struct OutlineContour* oc) {
    free(oc->segments);
}

void outline_free(struct Outline* outline) {
    size_t i;

    for (i = 0; i < outline->numContours; i++) {
        outline_contour_free(outline->contours + i);
    }
    free(outline->contours);
}
