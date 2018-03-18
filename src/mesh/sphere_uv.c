#include <stdlib.h>
#include <math.h>
#include "sphere_uv.h"

#define central_cylindrical tan

static double mercator(double latitude) {
    return log(tan((M_PI / 4.0) + (latitude / 2.0)));
}

static double miller(double latitude) {
    return (5.0 / 4.0) * log(tan((M_PI / 4.0) + (2.0 * latitude / 5.0)));
}

static double equirectangular(double latitude) {
    return latitude;
}

static double (*latitude_to_y[NUM_SPHERE_MAP])(double latitude) = {
    central_cylindrical,
    mercator,
    miller,
    equirectangular
};

static void get_longitude_latidude(float* p, double* longitude, double* latitude) {
    double r = sqrt(p[0] * p[0] + p[1] * p[1] + p[2] * p[2]);
    *longitude = atan2(p[1], p[0]);
    *latitude = (M_PI / 2.0) - acos(p[2] / r);
}

static void fix(float* u, double texWidth, void (*update)(struct Mesh*, unsigned int, float), struct Mesh* mesh, unsigned int i) {
    int wa, wb, wc;
    unsigned int w;

    if ((u[0] < 0.0) + (u[1] < 0.0) + (u[2] < 0.0)) {
        return;
    }
    wa = fabs(u[0] - u[1]) > (texWidth / 2.0);
    wb = fabs(u[1] - u[2]) > (texWidth / 2.0);
    wc = fabs(u[2] - u[0]) > (texWidth / 2.0);
    if (!(wa + wb + wc)) {
        return;
    }
    if (wa && wb) {
        w = 1;
    } else if (wa && wc) {
        w = 0;
    } else if (wb && wc) {
        w = 2;
    } else {
        return;
    }
    update(mesh, i + w, (u[w] > (texWidth / 2.0)) ? (u[w] - texWidth) : (u[w] + texWidth));
}

static void update_indexed(struct Mesh* mesh, unsigned int i, float val) {
    unsigned int newIndex;
    if (!(newIndex = mesh_duplicate_index(mesh, mesh->indices[i]))) {
        return;
    }
    mesh->indices[i] = newIndex;
    mesh->texCoords[2 * newIndex] = val;
}

static void update_continuous(struct Mesh* mesh, unsigned int i, float val) {
    mesh->texCoords[2 * i] = val;
}

int compute_sphere_uv(struct Mesh* sphere, double texWidth, double texHeight, double texRatio, enum SphereMapType type) {
    double y, yMax = M_PI * (texHeight / texWidth) / texRatio;
    double longitude, latitude;
    unsigned int i;
    float* vertex = sphere->vertices;
    float *uv, u[3];

    if (!(uv = malloc(2 * sphere->numVertices * sizeof(float)))) {
        return 0;
    }
    sphere->texCoords = uv;
    sphere->hasTexCoords = 1;

    for (i = 0; i < sphere->numVertices; i++) {
        get_longitude_latidude(vertex, &longitude, &latitude);
        y = latitude_to_y[type](latitude);
        if (fabs(y) > yMax) {
            uv[0] = -1.0f;
            uv[1] = -1.0f;
        } else {
            uv[0] = (longitude + M_PI) / (2.0 * M_PI) * texWidth;
            uv[1] = 1.0 - ((1.0 - (y / yMax)) / 2.0) * texHeight;
        }
        vertex += 3;
        uv += 2;
    }

    /* Ensure all "u"s of each face are on "the same side" */
    if (sphere->numIndices) {
        for (i = 0; i < sphere->numIndices; i += 3) {
            u[0] = sphere->texCoords[2 * sphere->indices[i]];
            u[1] = sphere->texCoords[2 * sphere->indices[i + 1]];
            u[2] = sphere->texCoords[2 * sphere->indices[i + 2]];
            fix(u, texWidth, update_indexed, sphere, i);
        }
    } else {
        for (i = 0; i < sphere->numVertices; i += 3) {
            u[0] = sphere->texCoords[2 * (i)];
            u[1] = sphere->texCoords[2 * (i + 1)];
            u[2] = sphere->texCoords[2 * (i + 2)];
            fix(u, texWidth, update_continuous, sphere, i);
        }
    }

    return 1;
}
