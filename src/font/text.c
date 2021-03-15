#include <stdlib.h>
#include <limits.h>
#include <float.h>
#include <3dmr/font/text.h>
#include <3dmr/math/interp.h>
#include <3dmr/math/polynom.h>

void character_free(struct Character* c) {
    outline_free(&c->outline);
}

struct CP {
    float x;
    int d;
};

static struct CP* cp_append(struct CP** cps, size_t* ncp, size_t* acp) {
    struct CP* cp;
    size_t n;

    if (!(n = *ncp + 1U)) return NULL;
    if (n <= *acp) {
        cp = *cps;
    } else {
        if (n > ((size_t)-1) / sizeof(**cps) || !(cp = realloc(*cps, n * sizeof(**cps)))) {
            return NULL;
        }
        *cps = cp;
        *acp = n;
    }
    *ncp = n;
    return cp + (n - 1U);
}

static int cp_cmp(const void* a, const void* b) {
    const struct CP* cpa = a;
    const struct CP* cpb = b;
    return (cpa->x > cpb->x) - (cpa->x < cpb->x);
}

GLuint text_to_sdm_texture(const struct Character* chars, size_t numChars, size_t mapHeight, size_t* mapWidth) {
    float* sdm;
    float* mindist;
    unsigned char* sdm8;
    unsigned char* sdm8dest;
    struct CL {
        float xoffset; /* in font units */
        size_t xmin, xmax; /* in pixels */
    } *cl, *clcur[3];
    struct CP* cps = NULL;
    const struct CP* cplast;
    const struct CP* cpcur;
    const struct CP* cpend;
    const struct Outline* outline;
    float coefs[3], roots[2];
    Vec2 pt, pt2;
    float cyMin, cyMax, xMin, yMin, yMax, scale, scaleinv, maxdist = 0;
    float pen, width, a, more, lsb;
    size_t i, j, k, x, y, w, npix, ncp, acp = 0;
    GLuint tex;
    int winding, r, numRoots;

    if (!numChars) return 0;
    yMin = chars[0].yMin;
    yMax = chars[0].yMax;
    if (yMax < yMin) return 0;
    if ((a = chars[0].advance) < 0.0f) return 0;
    width = chars[0].xMax - (xMin = chars[0].xMin) + (lsb = chars[0].lsb);
    if (width < a) width = a;
    if (lsb < 0.0f) {
        pen = -lsb;
        width += pen;
    } else {
        pen = 0;
    }
    if (numChars > ((size_t)-1) / sizeof(*cl) || !(cl = malloc(numChars * sizeof(*cl)))) return 0;
    cl[0].xoffset = pen + lsb - xMin;
    pen += a;
    for (i = 1; i < numChars; i++) {
        if ((cyMin = chars[i].yMin) < yMin) yMin = cyMin;
        if ((cyMax = chars[i].yMax) > yMax) yMax = cyMax;
        if (cyMax < cyMin || (a = chars[i].advance) < 0.0f || ((lsb = chars[i].lsb) < 0.0f && lsb + width < 0.0f)) {
            free(cl);
            return 0;
        }
        more = chars[i].xMax - (xMin = chars[i].xMin) + lsb;
        if (more < a) more = a;
        if (pen + more > width) width = pen + more;
        cl[i].xoffset = pen + lsb - xMin;
        pen += a;
    }
    scale = ((float)mapHeight) / (yMax - yMin);
    scaleinv = 1.0f / scale;
    w = width * scale;
    if (mapWidth) *mapWidth = w;
    for (i = 0; i < numChars; i++) {
        cl[i].xmin = (cl[i].xoffset + chars[i].xMin) * scale;
        cl[i].xmax = (cl[i].xoffset + chars[i].xMax) * scale;
    }
    if (w > ((size_t)-1) / mapHeight || (npix = w * mapHeight) > ((size_t)-1) - sizeof(*sdm)
     || !(sdm = malloc(npix * sizeof(*sdm)))) {
        free(cl);
        return 0;
    }
    for (y = 0, mindist = sdm; y < mapHeight; y++) {
        clcur[0] = clcur[1] = NULL;
        clcur[2] = cl;
        pt[1] = pt2[1] = y * scaleinv + yMin;
        ncp = 0;
        for (i = 0; i < numChars; i++) {
            outline = &chars[i].outline;
            for (j = 0; j < outline->numContours; j++) {
                for (k = 0; k < outline->contours[j].numSegments; k++) {
                    const union OutlineSegment *const seg = outline->contours[j].segments + k;
                    struct CP* cp;
                    int ok = 1;

                    switch (seg->type) {
                        case OUTLINE_SEGMENT_LINEAR:
                            coefs[0] = seg->linear.p0[1] - pt[1];
                            coefs[1] = seg->linear.p1[1] - seg->linear.p0[1];
                            if ((numRoots = polynom_roots_1(roots, coefs)) > 0 && roots[0] >= 0.0f && roots[0] <= 1.0f) {
                                if ((ok = (cp = cp_append(&cps, &ncp, &acp)) != NULL)) {
                                    cp->x = seg->linear.p0[0] + roots[0] * (seg->linear.p1[0] - seg->linear.p0[0]) + cl[i].xoffset;
                                    cp->d = 2 * (coefs[1] > 0.0f) - 1;
                                }
                            }
                            break;
                        case OUTLINE_SEGMENT_QUADRATIC:
                            if ((pt[1] > seg->quadratic.p0[1] && pt[1] > seg->quadratic.p1[1] && pt[1] > seg->quadratic.p2[1])
                             || (pt[1] < seg->quadratic.p0[1] && pt[1] < seg->quadratic.p1[1] && pt[1] < seg->quadratic.p2[1])) {
                                break;
                            } else {
                                float dy01 = seg->quadratic.p1[1] - seg->quadratic.p0[1];
                                float dy12 = seg->quadratic.p2[1] - seg->quadratic.p1[1];
                                float d;
                                coefs[0] = seg->quadratic.p0[1] - pt[1];
                                coefs[1] = 2.0f * dy01;
                                coefs[2] = dy12 - dy01;
                                numRoots = polynom_roots_2(roots, coefs);
                                for (r = 0; ok && r < numRoots; r++) {
                                    if (roots[r] >= 0.0f && roots[r] <= 1.0f
                                     && (d = ((1.0f - roots[r]) * coefs[1] + 2.0f * roots[r] * dy12))
                                     && (ok = (cp = cp_append(&cps, &ncp, &acp)) != NULL)) {
                                        cp->x = seg->quadratic.p0[0]
                                              + 2.0f * roots[r] * (seg->quadratic.p1[0] - seg->quadratic.p0[0])
                                              + roots[r] * roots[r] * (seg->quadratic.p0[0] - 2.0f * seg->quadratic.p1[0] + seg->quadratic.p2[0])
                                              + cl[i].xoffset;
                                        cp->d = 2 * (d > 0.0f) - 1;
                                    }
                                }
                            }
                            break;
                    }
                    if (!ok) {
                        free(cl);
                        free(cps);
                        free(sdm);
                        return 0;
                    }
                }
            }
        }
        qsort(cps, ncp, sizeof(*cps), cp_cmp);
        cplast = NULL;
        cpcur = cps;
        cpend = cps + ncp;
        winding = 0;
        for (x = 0; x < w; x++, mindist++) {
            pt[0] = x * scaleinv;
            while (cpcur < cpend && pt[0] > cpcur->x) {
                if (!cplast || cpcur->x - cplast->x > 1e-3 || cpcur->d != cplast->d) {
                    winding += cpcur->d;
                    if (winding == INT_MAX || winding == INT_MIN) {
                        free(cl);
                        free(cps);
                        free(sdm);
                        return 0;
                    }
                }
                cplast = cpcur++;
            }
            if (clcur[1] && x > clcur[1]->xmax) {
                clcur[0] = clcur[1];
                clcur[1] = NULL;
            }
            if (!clcur[1] && clcur[2] && x >= clcur[2]->xmin) {
                clcur[1] = clcur[2]++;
                if (clcur[2] - cl >= numChars) {
                    clcur[2] = NULL;
                }
            }
            *mindist = FLT_MAX;
            for (i = 0; i < 3U; i++) {
                if (!clcur[i]) continue;
                outline = &chars[clcur[i] - cl].outline;
                pt2[0] = pt[0] - clcur[i]->xoffset;
                for (j = 0; j < outline->numContours; j++) {
                    for (k = 0; k < outline->contours[j].numSegments; k++) {
                        float dist = distance_point_to_outline_segment(pt2, outline->contours[j].segments + k);
                        if (dist < *mindist) *mindist = dist;
                    }
                }
            }
            if (!winding) { /* truetype use non-zero winding rule */
                *mindist = -*mindist;
            } else if (*mindist > maxdist) {
                maxdist = *mindist;
            }
        }
    }
    free(cl);
    free(cps);
    if (!(sdm8 = malloc(npix))) {
        free(sdm);
        return 0;
    }
    scale = 127.5f / maxdist;
    for (y = 0, mindist = sdm, sdm8dest = sdm8; y < mapHeight; y++) {
        for (x = 0; x < w; x++, mindist++, sdm8dest++) {
            *sdm8dest = clamp(*mindist * scale + 127.5f, 0.0f, 255.0f);
        }
    }
    free(sdm);
    tex = texture_load_from_uchar_buffer(sdm8, w, mapHeight, 1, 1);
    free(sdm8);
    return tex;
}
