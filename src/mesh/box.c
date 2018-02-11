#include <stdlib.h>
#include <string.h>
#include "box.h"

static const float normals[] = {
     0,  0, -1,    0,  0, -1,    0,  0, -1,    0,  0, -1,    0,  0, -1,    0,  0, -1,
     0,  0,  1,    0,  0,  1,    0,  0,  1,    0,  0,  1,    0,  0,  1,    0,  0,  1,
    -1,  0,  0,   -1,  0,  0,   -1,  0,  0,   -1,  0,  0,   -1,  0,  0,   -1,  0,  0,
     1,  0,  0,    1,  0,  0,    1,  0,  0,    1,  0,  0,    1,  0,  0,    1,  0,  0,
     0, -1,  0,    0, -1,  0,    0, -1,  0,    0, -1,  0,    0, -1,  0,    0, -1,  0,
     0,  1,  0,    0,  1,  0,    0,  1,  0,    0,  1,  0,    0,  1,  0,    0,  1,  0
};

static const float texcoords[] = {
    0, 1,    1, 1,    1, 0,    0, 1,    0, 0,    1, 0,
    0, 1,    1, 1,    1, 0,    0, 1,    0, 0,    1, 0,
    0, 1,    1, 1,    1, 0,    0, 1,    0, 0,    1, 0,
    0, 1,    1, 1,    1, 0,    0, 1,    0, 0,    1, 0,
    0, 1,    1, 1,    1, 0,    0, 1,    0, 0,    1, 0,
    0, 1,    1, 1,    1, 0,    0, 1,    0, 0,    1, 0
};


int make_box(struct Mesh* dest, float width, float height, float depth) {
    float a = width / 2.0f, b = height / 2.0f, c = depth / 2.0f;

    dest->numVertices = 36;
    dest->numNormals = 36;
    dest->numTexCoords = 36;
    dest->numIndices = 0;

    dest->vertices = malloc(sizeof(normals));
    dest->normals = malloc(sizeof(normals));
    dest->texCoords = malloc(sizeof(texcoords));
    dest->indices = NULL;
    if (!dest->vertices || !dest->normals || !dest->texCoords) {
        free(dest->vertices);
        free(dest->normals);
        free(dest->texCoords);
        return 0;
    }

    dest->vertices[  0] = -a; dest->vertices[  1] = -b; dest->vertices[  2] = -c;
    dest->vertices[  3] =  a; dest->vertices[  4] = -b; dest->vertices[  5] = -c;
    dest->vertices[  6] =  a; dest->vertices[  7] =  b; dest->vertices[  8] = -c;
    dest->vertices[  9] = -a; dest->vertices[ 10] = -b; dest->vertices[ 11] = -c;
    dest->vertices[ 12] = -a; dest->vertices[ 13] =  b; dest->vertices[ 14] = -c;
    dest->vertices[ 15] =  a; dest->vertices[ 16] =  b; dest->vertices[ 17] = -c;
    dest->vertices[ 18] = -a; dest->vertices[ 19] = -b; dest->vertices[ 20] =  c;
    dest->vertices[ 21] =  a; dest->vertices[ 22] = -b; dest->vertices[ 23] =  c;
    dest->vertices[ 24] =  a; dest->vertices[ 25] =  b; dest->vertices[ 26] =  c;
    dest->vertices[ 27] = -a; dest->vertices[ 28] = -b; dest->vertices[ 29] =  c;
    dest->vertices[ 30] = -a; dest->vertices[ 31] =  b; dest->vertices[ 32] =  c;
    dest->vertices[ 33] =  a; dest->vertices[ 34] =  b; dest->vertices[ 35] =  c;
    dest->vertices[ 36] = -a; dest->vertices[ 37] = -b; dest->vertices[ 38] = -c;
    dest->vertices[ 39] = -a; dest->vertices[ 40] =  b; dest->vertices[ 41] = -c;
    dest->vertices[ 42] = -a; dest->vertices[ 43] =  b; dest->vertices[ 44] =  c;
    dest->vertices[ 45] = -a; dest->vertices[ 46] = -b; dest->vertices[ 47] = -c;
    dest->vertices[ 48] = -a; dest->vertices[ 49] = -b; dest->vertices[ 50] =  c;
    dest->vertices[ 51] = -a; dest->vertices[ 52] =  b; dest->vertices[ 53] =  c;
    dest->vertices[ 54] =  a; dest->vertices[ 55] = -b; dest->vertices[ 56] = -c;
    dest->vertices[ 57] =  a; dest->vertices[ 58] =  b; dest->vertices[ 59] = -c;
    dest->vertices[ 60] =  a; dest->vertices[ 61] =  b; dest->vertices[ 62] =  c;
    dest->vertices[ 63] =  a; dest->vertices[ 64] = -b; dest->vertices[ 65] = -c;
    dest->vertices[ 66] =  a; dest->vertices[ 67] = -b; dest->vertices[ 68] =  c;
    dest->vertices[ 69] =  a; dest->vertices[ 70] =  b; dest->vertices[ 71] =  c;
    dest->vertices[ 72] = -a; dest->vertices[ 73] = -b; dest->vertices[ 74] = -c;
    dest->vertices[ 75] =  a; dest->vertices[ 76] = -b; dest->vertices[ 77] = -c;
    dest->vertices[ 78] =  a; dest->vertices[ 79] = -b; dest->vertices[ 80] =  c;
    dest->vertices[ 81] = -a; dest->vertices[ 82] = -b; dest->vertices[ 83] = -c;
    dest->vertices[ 84] = -a; dest->vertices[ 85] = -b; dest->vertices[ 86] =  c;
    dest->vertices[ 87] =  a; dest->vertices[ 88] = -b; dest->vertices[ 89] =  c;
    dest->vertices[ 90] = -a; dest->vertices[ 91] =  b; dest->vertices[ 92] = -c;
    dest->vertices[ 93] =  a; dest->vertices[ 94] =  b; dest->vertices[ 95] = -c;
    dest->vertices[ 96] =  a; dest->vertices[ 97] =  b; dest->vertices[ 98] =  c;
    dest->vertices[ 99] = -a; dest->vertices[100] =  b; dest->vertices[101] = -c;
    dest->vertices[102] = -a; dest->vertices[103] =  b; dest->vertices[104] =  c;
    dest->vertices[105] =  a; dest->vertices[106] =  b; dest->vertices[107] =  c;

    memcpy(dest->normals, normals, sizeof(normals));
    memcpy(dest->texCoords, texcoords, sizeof(texcoords));

    return 1;
}
