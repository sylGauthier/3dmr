#include "globject.h"

void globject_new(const struct Mesh* mesh, struct GLObject* glo) {
    unsigned int haveNormals = !!mesh->numNormals;
    unsigned int haveTexCoords = !!mesh->numTexCoords;
    unsigned int haveIndices = !!mesh->numIndices;

    glo->numVertices = mesh->numVertices;
    glo->numIndices = mesh->numIndices;

    /* VBOs */
    glo->numVBOs = 1 + haveNormals + haveTexCoords + haveIndices;
    glGenBuffers(glo->numVBOs, glo->vbo);
    /* - Vertices */
    glBindBuffer(GL_ARRAY_BUFFER, glo->vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, mesh->numVertices * 3 * sizeof(float), mesh->vertices, GL_STATIC_DRAW);
    /* - Normals */
    if (haveNormals) {
        glBindBuffer(GL_ARRAY_BUFFER, glo->vbo[1]);
        glBufferData(GL_ARRAY_BUFFER, mesh->numNormals * 3 * sizeof(float), mesh->normals, GL_STATIC_DRAW);
    }
    /* - TexCoords */
    if (haveTexCoords) {
        glBindBuffer(GL_ARRAY_BUFFER, glo->vbo[1 + haveNormals]);
        glBufferData(GL_ARRAY_BUFFER, mesh->numTexCoords * 2 * sizeof(float), mesh->texCoords, GL_STATIC_DRAW);
    }
    /* - Indices */
    if (haveIndices) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glo->vbo[1 + haveNormals + haveTexCoords]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->numIndices * sizeof(unsigned int), mesh->indices, GL_STATIC_DRAW);
    }
    /* done */
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    /* VAO */
    glGenVertexArrays(1, &glo->vao);
    glBindVertexArray(glo->vao);
    /* - Vertices */
    glBindBuffer(GL_ARRAY_BUFFER, glo->vbo[0]);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    /* - Normals */
    if (haveNormals) {
        glBindBuffer(GL_ARRAY_BUFFER, glo->vbo[1]);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    }
    /* - TexCoords */
    if (haveTexCoords) {
        glBindBuffer(GL_ARRAY_BUFFER, glo->vbo[1 + haveNormals]);
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
    }
    /* - Indices */
    if (haveIndices) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glo->vbo[1 + haveNormals + haveTexCoords]);
    }
    /* done */
    glBindVertexArray(0);
}

void globject_free(struct GLObject* glo) {
    if (glo->numVBOs) {
        glDeleteBuffers(glo->numVBOs, glo->vbo);
        glo->numVBOs = 0;
    }
    if (glo->vao && glIsVertexArray(glo->vao) == GL_TRUE) {
        glDeleteVertexArrays(1, &glo->vao);
    }
}
