#include <stdlib.h>
#include "vertex_array.h"
#include "shader.h"

void vertex_array_gen(const struct Mesh* mesh, struct VertexArray* va) {
    GLsizei stride = MESH_SIZEOF_VERTEX(mesh);
    float* offset = 0;
    unsigned int hasIndices = !!mesh->numIndices;

    va->numVertices = mesh->numVertices;
    va->numIndices = mesh->numIndices;

    /* VBO */
    glGenBuffers(1, &va->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, va->vbo);
    glBufferData(GL_ARRAY_BUFFER, MESH_SIZEOF_VERTICES(mesh), mesh->vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    /* IBO */
    if (hasIndices) {
        glGenBuffers(1, &va->ibo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, va->ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->numIndices * sizeof(unsigned int), mesh->indices, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    } else {
        va->ibo = 0;
    }

    /* VAO */
    glGenVertexArrays(1, &va->vao);
    glBindVertexArray(va->vao);
    /* - Vertices */
    glBindBuffer(GL_ARRAY_BUFFER, va->vbo);
    glEnableVertexAttribArray(LOCATION_VERTEX);
    glVertexAttribPointer(LOCATION_VERTEX, 3, GL_FLOAT, GL_FALSE, stride, (void*)offset);
    offset += 3;
    /* - Normals */
    if (MESH_HAS_NORMALS(mesh)) {
        glEnableVertexAttribArray(LOCATION_NORMAL);
        glVertexAttribPointer(LOCATION_NORMAL, 3, GL_FLOAT, GL_FALSE, stride, (void*)offset);
        offset += 3;
    }
    /* - TexCoords */
    if (MESH_HAS_TEXCOORDS(mesh)) {
        glEnableVertexAttribArray(LOCATION_TEXCOORD);
        glVertexAttribPointer(LOCATION_TEXCOORD, 2, GL_FLOAT, GL_FALSE, stride, (void*)offset);
        offset += 2;
    }
    /* - Tangents and bitangents */
    if (MESH_HAS_TANGENTS(mesh)) {
        glEnableVertexAttribArray(LOCATION_TANGENT);
        glVertexAttribPointer(LOCATION_TANGENT, 3, GL_FLOAT, GL_FALSE, stride, (void*)offset);
        offset += 3;
        glEnableVertexAttribArray(LOCATION_BITANGENT);
        glVertexAttribPointer(LOCATION_BITANGENT, 3, GL_FLOAT, GL_FALSE, 0, (void*)offset);
        offset += 3;
    }
    /* - Indices */
    if (hasIndices) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, va->ibo);
    }
    glBindVertexArray(0);
}

void vertex_array_del(struct VertexArray* va) {
    if (va->vbo) {
        glDeleteBuffers(1, &va->vbo);
        va->vbo = 0;
    }
    if (va->ibo) {
        glDeleteBuffers(1, &va->ibo);
        va->ibo = 0;
    }
    if (va->vao) {
        glDeleteVertexArrays(1, &va->vao);
        va->vao = 0;
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
