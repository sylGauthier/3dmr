#include <stdlib.h>
#include <string.h>
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

static void get_longitude_latidude(double p0, double p1, double p2, double* longitude, double* latitude) {
    double r = sqrt(p0 * p0 + p1 * p1 + p2 * p2);
    *longitude = atan2(p0, p2);
    *latitude = (M_PI / 2.0) - acos(p1 / r);
}

#define VINDEX(mesh, nFace, nVertex) (mesh->numIndices ? mesh->indices[3 * nFace + nVertex] : (3 * nFace + nVertex))
#define GET_FACE_VERTEX(mesh, nFace, nVertex) (mesh->vertices + MESH_FLOATS_PER_VERTEX(mesh) * VINDEX(mesh, nFace, nVertex))
#define GET_FACE_UV(mesh, nFace, nVertex) (mesh->vertices + MESH_FLOATS_PER_VERTEX(mesh) * VINDEX(mesh, nFace, nVertex) + (MESH_HAS_NORMALS(mesh) ? 6 : 3))

int compute_sphere_uv(struct Mesh* sphere, double texWidth, double texHeight, double texRatio, enum SphereMapType type) {
    double y, yMax = M_PI * (texHeight / texWidth) / texRatio;
    double longitude, latitude;
    unsigned int i, j, k, w, o, n, N, M, a, b, numFaces = (sphere->numIndices ? sphere->numIndices : sphere->numVertices) / 3;
    float *vertex, *uv[3], u[3], v[3];

    if (MESH_HAS_TEXCOORDS(sphere) || MESH_HAS_TANGENTS(sphere)) {
        return 0;
    }
    M = MESH_FLOATS_PER_VERTEX(sphere);
    sphere->flags |= MESH_TEXCOORDS;
    N = MESH_FLOATS_PER_VERTEX(sphere);
    if (!(vertex = malloc(N * sphere->numVertices * sizeof(float)))) {
        return 0;
    }
    for (i = 0; i < sphere->numVertices; i++) {
        memcpy(vertex + N * i, sphere->vertices + M * i, M * sizeof(float));
    }
    free(sphere->vertices);
    sphere->vertices = vertex;
    sphere->flags |= MESH_TEXCOORDS;

    for (i = 0; i < numFaces; i++) {
        o = 0;
        for (j = 0; j < 3; j++) {
            vertex = GET_FACE_VERTEX(sphere, i, j);
            uv[j] = GET_FACE_UV(sphere, i, j);
            get_longitude_latidude(vertex[0], vertex[1], vertex[2], &longitude, &latitude);
            y = latitude_to_y[type](latitude);
            o |= (fabs(y) > yMax) << j;
            y = (y > yMax) ? yMax : (y < -yMax) ? -yMax : y;
            u[j] = (longitude + M_PI) / (2.0 * M_PI) * texWidth;
            v[j] = 1.0 - ((1.0 - (y / yMax)) / 2.0) * texHeight;
        }
        if (o) {
            k = o >> 1;
            if ((fabs(u[(k + 1) % 3] - u[(k + 2) % 3]) > (texWidth / 2.0))) {
                a = (u[(k + 1) % 3] > (texWidth / 2.0)) ? (texWidth - u[(k + 1) % 3]) : u[(k + 1) % 3];
                b = (u[(k + 2) % 3] > (texWidth / 2.0)) ? (texWidth - u[(k + 2) % 3]) : u[(k + 2) % 3];
                w = 1 << ((k + 1 + (a < b)) % 3);
            } else {
                w = 7;
            }
        } else {
            w = ((fabs(u[1] - u[2]) < (texWidth / 2.0)))
              | ((fabs(u[2] - u[0]) < (texWidth / 2.0)) << 1)
              | ((fabs(u[0] - u[1]) < (texWidth / 2.0)) << 2);
        }
        for (j = 0; j < 3; j++) {
            if ((w == (1 << j))) {
                u[j] = (u[j] > (texWidth / 2.0)) ? (u[j] - texWidth) : (u[j] + texWidth);
            }
        }
        for (j = 0; j < 3; j++) {
            if ((o == (1 << j))) {
                u[j] = (u[(j + 1) % 3] + u[(j + 2) % 3]) / 2.0;
            }
        }
        for (j = 0; j < 3; j++) {
            if (sphere->numIndices && ((w == (1 << j)) | (o == (1 << j)))) {
                if ((n = mesh_duplicate_index(sphere, sphere->indices[k = (3 * i + j)]))) {
                    sphere->indices[k] = n;
                    uv[j] = sphere->vertices + N * n + (MESH_HAS_NORMALS(sphere) ? 6 : 3);
                }
            }
            uv[j][0] = u[j];
            uv[j][1] = v[j];
        }
    }

    return 1;
}
