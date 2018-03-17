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

int compute_sphere_uv(struct Mesh* sphere, double texWidth, double texHeight, double texRatio, enum SphereMapType type) {
    double y, yMax = M_PI * (texHeight / texWidth) / texRatio;
    double longitude, latitude;
    unsigned int i;
    float* vertex = sphere->vertices;
    float* uv;

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
            uv[1] = ((1.0 - (y / yMax)) / 2.0) * texHeight;
        }
        vertex += 3;
        uv += 2;
    }
    return 1;
}
