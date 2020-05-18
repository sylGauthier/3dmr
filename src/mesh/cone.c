#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <3dmr/math/linear_algebra.h>
#include <3dmr/mesh/cone.h>

int make_cone(struct Mesh* dest, float radius1, float radius2, float depth, unsigned int numSidePoints) {
    Vec3 d, r;
    float *c, *cf;
    double angle;
    unsigned int i, *ci, stride;
    int fDown = (radius1 > 0), fUp = (radius2 > 0);

    if (numSidePoints <= 2 || radius1 < 0 || radius2 < 0 || (!radius1 && !radius2)) return 0;
    stride = 2 + fDown + fUp;
    dest->numVertices = stride * numSidePoints;
    dest->flags = MESH_NORMALS;
    dest->numIndices = 6 * numSidePoints + 3 * (numSidePoints - 2) * (fDown + fUp);

    dest->vertices = malloc(MESH_SIZEOF_VERTICES(dest));
    dest->indices = malloc(dest->numIndices * sizeof(unsigned int));
    if (!dest->vertices || !dest->indices) {
        free(dest->vertices);
        free(dest->indices);
        return 0;
    }

    depth /= 2.0;
    c = dest->vertices;
    ci = dest->indices;
    for (i = 0; i < numSidePoints; i++) {
        angle = 2.0 * M_PI * ((double)i) / ((double)numSidePoints);
        c[0] = radius1 * cos(angle);
        c[1] = -depth;
        c[2] = radius1 * sin(angle);
        c[6] = radius2 * cos(angle);
        c[7] = depth;
        c[8] = radius2 * sin(angle);
        sub3v(d, c + 6, c);
        normalize3(d);
        r[0] = sin(angle);
        r[1] = 0;
        r[2] = -cos(angle);
        cross3(c + 3, r, d);
        memcpy(c + 9, c + 3, sizeof(Vec3));
        cf = c + 12;
        if (fDown) {
            memcpy(cf, c, sizeof(Vec3));
            cf[3] = cf[5] = 0;
            cf[4] = -1;
            cf += 6;
        }
        if (fUp) {
            memcpy(cf, c + 6, sizeof(Vec3));
            cf[3] = cf[5] = 0;
            cf[4] = 1;
            cf += 6;
        }
        c = cf;
        ci[0] = stride * i;
        ci[1] = stride * ((i + 1) % numSidePoints) + 1;
        ci[2] = stride * ((i + 1) % numSidePoints);
        ci[3] = ci[1];
        ci[4] = ci[0];
        ci[5] = stride * i + 1;
        ci += 6;
    }
    if (fDown) {
        for (i = 1; i < numSidePoints - 1; i++) {
            ci[0] = 2;
            ci[1] = stride * i + 2;
            ci[2] = stride * (i + 1) + 2;
            ci += 3;
        }
    }
    if (fUp) {
        for (i = 1; i < numSidePoints - 1; i++) {
            ci[0] = 2 + fDown;
            ci[1] = stride * (i + 1) + 2 + fDown;
            ci[2] = stride * i + 2 + fDown;
            ci += 3;
        }
    }

    return 1;
}

int make_cone_textured(struct Mesh* dest, float radius1, float radius2, float depth, unsigned int numSidePoints) {
    Vec3 d, r;
    float *c, *cf;
    double angle;
    unsigned int i, *ci, stride;
    int fDown = (radius1 > 0), fUp = (radius2 > 0);

    if (numSidePoints <= 2 || radius1 < 0 || radius2 < 0 || (!radius1 && !radius2)) return 0;
    stride = 2 + fDown + fUp;
    dest->numVertices = stride * numSidePoints + 2;
    dest->flags = MESH_NORMALS | MESH_TEXCOORDS;
    dest->numIndices = 6 * numSidePoints + 3 * (numSidePoints - 2) * (fDown + fUp);

    dest->vertices = malloc(MESH_SIZEOF_VERTICES(dest));
    dest->indices = malloc(dest->numIndices * sizeof(unsigned int));
    if (!dest->vertices || !dest->indices) {
        free(dest->vertices);
        free(dest->indices);
        return 0;
    }

    depth /= 2.0;
    c = dest->vertices;
    ci = dest->indices;
    for (i = 0; i <= numSidePoints; i++) {
        angle = 2.0 * M_PI * ((double)i) / ((double)numSidePoints);
        c[0] = radius1 * cos(angle);
        c[1] = -depth;
        c[2] = radius1 * sin(angle);
        c[8] = radius2 * cos(angle);
        c[9] = depth;
        c[10] = radius2 * sin(angle);
        sub3v(d, c + 8, c);
        normalize3(d);
        r[0] = sin(angle);
        r[1] = 0;
        r[2] = -cos(angle);
        cross3(c + 3, r, d);
        memcpy(c + 11, c + 3, sizeof(Vec3));
        c[6] = c[14] = ((double)i) / ((double)numSidePoints);
        c[7] = 0.5;
        c[15] = 0.0;
        cf = c + 16;
        if (i < numSidePoints) {
            if (fDown) {
                memcpy(cf, c, sizeof(Vec3));
                cf[3] = cf[5] = 0;
                cf[4] = -1;
                cf[6] = 0.25 * (cos(angle) + 1.0);
                cf[7] = 0.25 * (sin(angle) + 3.0);
                cf += 8;
            }
            if (fUp) {
                memcpy(cf, c + 8, sizeof(Vec3));
                cf[3] = cf[5] = 0;
                cf[4] = 1;
                cf[6] = 0.25 * (cos(angle) + 3.0);
                cf[7] = 0.25 * (sin(angle) + 3.0);
                cf += 8;
            }
            c = cf;
            ci[0] = stride * i;
            ci[1] = stride * (i + 1) + 1;
            ci[2] = stride * (i + 1);
            ci[3] = ci[1];
            ci[4] = ci[0];
            ci[5] = stride * i + 1;
            ci += 6;
        }
    }
    if (fDown) {
        for (i = 1; i < numSidePoints - 1; i++) {
            ci[0] = 2;
            ci[1] = stride * i + 2;
            ci[2] = stride * (i + 1) + 2;
            ci += 3;
        }
    }
    if (fUp) {
        for (i = 1; i < numSidePoints - 1; i++) {
            ci[0] = 2 + fDown;
            ci[1] = stride * (i + 1) + 2 + fDown;
            ci[2] = stride * i + 2 + fDown;
            ci += 3;
        }
    }

    return 1;
}
