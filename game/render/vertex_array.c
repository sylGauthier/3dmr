#include <stdlib.h>
#include "vertex_array.h"

void vertex_array_gen(const struct Mesh* mesh, struct VertexArray* va) {
    unsigned int hasNormals = !!mesh->hasNormals;
    unsigned int hasTexCoords = !!mesh->hasTexCoords;
    unsigned int hasIndices = !!mesh->numIndices;

    va->numVertices = mesh->numVertices;
    va->numIndices = mesh->numIndices;

    /* VBOs */
    va->numVBOs = 1 + hasNormals + hasTexCoords + hasIndices;
    glGenBuffers(va->numVBOs, va->vbo);
    /* - Vertices */
    glBindBuffer(GL_ARRAY_BUFFER, va->vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, mesh->numVertices * 3 * sizeof(float), mesh->vertices, GL_STATIC_DRAW);
    /* - Normals */
    if (hasNormals) {
        glBindBuffer(GL_ARRAY_BUFFER, va->vbo[1]);
        glBufferData(GL_ARRAY_BUFFER, mesh->numVertices * 3 * sizeof(float), mesh->normals, GL_STATIC_DRAW);
    }
    /* - TexCoords */
    if (hasTexCoords) {
        glBindBuffer(GL_ARRAY_BUFFER, va->vbo[1 + hasNormals]);
        glBufferData(GL_ARRAY_BUFFER, mesh->numVertices * 2 * sizeof(float), mesh->texCoords, GL_STATIC_DRAW);
    }
    /* - Indices */
    if (hasIndices) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, va->vbo[1 + hasNormals + hasTexCoords]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->numIndices * sizeof(unsigned int), mesh->indices, GL_STATIC_DRAW);
    }
    /* done */
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    /* VAO */
    glGenVertexArrays(1, &va->vao);
    glBindVertexArray(va->vao);
    /* - Vertices */
    glBindBuffer(GL_ARRAY_BUFFER, va->vbo[0]);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    /* - Normals */
    if (hasNormals) {
        glBindBuffer(GL_ARRAY_BUFFER, va->vbo[1]);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    }
    /* - TexCoords */
    if (hasTexCoords) {
        glBindBuffer(GL_ARRAY_BUFFER, va->vbo[1 + hasNormals]);
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
    }
    /* - Indices */
    if (hasIndices) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, va->vbo[1 + hasNormals + hasTexCoords]);
    }
    /* done */
    glBindVertexArray(0);
}

void vertex_array_del(struct VertexArray* va) {
    if (va->numVBOs) {
        glDeleteBuffers(va->numVBOs, va->vbo);
        va->numVBOs = 0;
    }
    if (va->vao && glIsVertexArray(va->vao) == GL_TRUE) {
        glDeleteVertexArrays(1, &va->vao);
    }
}

struct VertexArray* vertex_array_new(const struct Mesh* mesh) {
    struct VertexArray* va;

    if ((va = malloc(sizeof(*va)))) {
        vertex_array_gen(mesh, va);
    }

    return va;
}

void vertex_array_free(struct VertexArray* va) {
    if (va) {
        vertex_array_del(va);
        free(va);
    }
}
