#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "obj.h"

static void obj_parse_face_elem(struct OBJFaceElem* elem, const char** line) {
    elem->v = strtoul(*line, (char**)line, 10) - 1;
    elem->t = (**line == '/') ? (strtoul(*line + 1, (char**)line, 10) - 1) : 0;
    elem->n = (**line == '/') ? (strtoul(*line + 1, (char**)line, 10) - 1) : 0;
}

static int obj_parse_line(struct OBJ* obj, const char* line) {
    float **t, *tmp;
    struct OBJFace* tmpFace;
    struct OBJFaceElem* tmpElem;
    unsigned int *n, i, s, alloc;
    const char* old;

    if (*line == 'v') {
        if (isspace(*(++line))) {
            t = &obj->vertices;
            n = &obj->numVertices;
            s = 3;
        } else if (*line == 'n' && isspace(line[1])) {
            t = &obj->normals;
            n = &obj->numNormals;
            s = 3;
            line++;
        } else if (*line == 't' && isspace(line[1])) {
            t = &obj->texCoords;
            n = &obj->numTexCoords;
            s = 2;
            line++;
        } else {
            return 1;
        }
        alloc = (*n) + 32 - (*n % 32);
        if (*n < alloc) {
            if (!(tmp = realloc(*t, alloc * s * sizeof(float)))) return 0;
            *t = tmp;
        }
        tmp = (*t) + (*n)++ * s;
        for (i = 0; i < s; i++) {
            *tmp++ = strtod(line, (char**)&line);
        }
    } else if (*line == 'f' && isspace(line[1])) {
        alloc = obj->numFaces + 32 - (obj->numFaces % 32);
        if (obj->numFaces < alloc) {
            if (!(tmpFace = realloc(obj->faces, alloc * sizeof(struct OBJFace)))) return 0;
            obj->faces = tmpFace;
        }
        tmpFace = obj->faces + obj->numFaces++;
        tmpFace->elems = NULL;
        tmpFace->numElems = 0;
        line++;
        while (*line) {
            alloc = tmpFace->numElems + 4 - (tmpFace->numElems % 4);
            if (tmpFace->numElems < alloc) {
                if (!(tmpElem = realloc(tmpFace->elems, alloc * sizeof(struct OBJFaceElem)))) return 0;
                tmpFace->elems = tmpElem;
            }
            old = line;
            obj_parse_face_elem(tmpFace->elems + tmpFace->numElems++, &line);
            if (old == line) break;
            while (isspace(*line)) line++;
        }
    }
    return 1;
}

int obj_load(struct OBJ* obj, FILE* file) {
    char l[2048];
    unsigned int i, j;
    int ret = 1;
    memset(obj, 0, sizeof(struct OBJ));
    while (ret && fgets(l, sizeof(l), file)) {
        ret = obj_parse_line(obj, l);
    }
    for (i = 0; ret && i < obj->numFaces; i++) {
        for (j = 0; ret && j < obj->faces[i].numElems; j++) {
            ret = obj->faces[i].elems[j].v < obj->numVertices
                && (!obj->numNormals || obj->faces[i].elems[j].n < obj->numNormals)
                && (!obj->numTexCoords || obj->faces[i].elems[j].t < obj->numTexCoords);
        }
    }
    if (!ret) {
        obj_free(obj);
    }
    return ret;
}

void obj_free(struct OBJ* obj) {
    free(obj->vertices);
    free(obj->normals);
    free(obj->texCoords);
    while (obj->numFaces--) {
        free(obj->faces[obj->numFaces].elems);
    }
    free(obj->faces);
}

int obj_triangulate(struct OBJ* obj) {
    struct OBJFace* faces;
    unsigned int i, j, k, numFaces;

    numFaces = 0;
    for (i = 0; i < obj->numFaces; i++) {
        if (obj->faces[i].numElems < 3) return 0;
        numFaces += obj->faces[i].numElems - 2;
    }
    if (!(faces = malloc(numFaces * sizeof(struct OBJFace)))) return 0;
    k = 0;
    for (i = 0; i < obj->numFaces; i++) {
        for (j = 2; j < obj->faces[i].numElems; j++) {
            if (!(faces[k].elems = malloc(3 * sizeof(struct OBJFaceElem)))) break;
            faces[k].numElems = 3;
            faces[k].elems[0] = obj->faces[i].elems[0];
            faces[k].elems[1] = obj->faces[i].elems[j - 1];
            faces[k].elems[2] = obj->faces[i].elems[j];
            k++;
        }
    }
    if (i == obj->numFaces) {
        while (obj->numFaces--) {
            free(obj->faces[obj->numFaces].elems);
        }
        free(obj->faces);
        obj->faces = faces;
        obj->numFaces = numFaces;
        return 1;
    }
    while (k--) {
        free(faces[k].elems);
    }
    free(faces);
    return 0;
}

int make_obj(struct Mesh* dest, const char* filename, int withIndices, int withNormals, int withTexCoords) {
    FILE* objFile = NULL;
    struct OBJ obj;
    unsigned int i, j, k, l;
    int objOk = 0, ret = 0;

    if (!(objFile = fopen(filename, "r"))) {
        fprintf(stderr, "Error: could not open obj file '%s'\n", filename);
    } else if (!(objOk = obj_load(&obj, objFile))) {
        fprintf(stderr, "Error: failed to parse obj file '%s'\n", filename);
    } else if (!obj_triangulate(&obj)) {
        fprintf(stderr, "Error: failed to triangulate obj file '%s'\n", filename);
    } else {
        if (!withIndices) {
            dest->numVertices = 3 * obj.numFaces;
            dest->numNormals = (withNormals && obj.numNormals) ? dest->numVertices : 0;
            dest->numTexCoords = (withTexCoords && obj.numTexCoords) ? dest->numVertices : 0;
            dest->numIndices = 0;
            dest->indices = NULL;
            dest->normals = NULL;
            dest->texCoords = NULL;
            if (!(dest->vertices = malloc(3 * dest->numVertices * sizeof(float)))
             || (dest->numNormals && !(dest->normals = malloc(3 * dest->numVertices * sizeof(float))))
             || (dest->numTexCoords && !(dest->texCoords = malloc(2 * dest->numVertices * sizeof(float))))) {
                fprintf(stderr, "Error: failed to allocate mesh buffer for obj file '%s'\n", filename);
                free(dest->vertices);
                free(dest->normals);
                free(dest->texCoords);
            } else {
                for (i = k = l = 0; i < obj.numFaces; i++) {
                    for (j = 0; j < 3; j++) {
                        memcpy(dest->vertices + k, obj.vertices + 3 * obj.faces[i].elems[j].v, 3 * sizeof(float));
                        if (dest->numNormals) memcpy(dest->normals + k, obj.normals + 3 * obj.faces[i].elems[j].n, 3 * sizeof(float));
                        if (dest->numTexCoords) memcpy(dest->texCoords + l, obj.texCoords + 2 * obj.faces[i].elems[j].t, 2 * sizeof(float));
                        k += 3;
                        l += 2;
                    }
                }
                ret = 1;
            }
        } else {
            dest->numIndices = 3 * obj.numFaces;
            if (!(dest->indices = malloc(dest->numIndices * sizeof(unsigned int)))) {
                fprintf(stderr, "Error: failed to allocate mesh buffers for obj file '%s'\n", filename);
            } else {
                dest->numVertices = obj.numVertices;
                dest->vertices = obj.vertices;
                obj.vertices = NULL;
                dest->numNormals = 0;
                dest->normals = NULL;
                dest->numTexCoords = 0;
                dest->texCoords = NULL;
                for (i = k = 0; i < obj.numFaces; i++) {
                    for (j = 0; j < 3; j++) {
                        dest->indices[k++] = obj.faces[i].elems[j].v;
                    }
                }
                ret = 1;
            }
        }
    }

    if (objOk) obj_free(&obj);
    if (objFile) fclose(objFile);
    return ret;
}
