#include "globject.h"

void globject_new(const struct Mesh* mesh, struct GLObject* glo) {
    unsigned int haveNormals = !!mesh->numNormals, haveTexCoords = !!mesh->numTexCoords;
    glo->numVertices = mesh->numVertices;

    /* VBOs */
    glo->numVBOs = 1 + haveNormals + haveTexCoords;
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
    glBindBuffer(GL_ARRAY_BUFFER, glo->vbo[1]);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    /* - TexCoords */
    glBindBuffer(GL_ARRAY_BUFFER, glo->vbo[2]);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
    /* done */
    glBindVertexArray(0);
}

void globject_free(struct GLObject* glo) {
    unsigned int i;
    glDeleteBuffers(glo->numVBOs, glo->vbo);
    glo->numVBOs = 0;
    if (glIsVertexArray(glo->vao) == GL_TRUE) {
        glDeleteVertexArrays(1, &glo->vao);
    }
}
