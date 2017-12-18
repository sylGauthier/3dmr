#include "globject.h"

void globject_new(const struct Mesh* mesh, struct GLObject* glo) {
    glo->numVertices = mesh->numVertices;

    /* VBOs */
    glGenBuffers(3, glo->vbo);
    /* - Vertices */
    glBindBuffer(GL_ARRAY_BUFFER, glo->vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, mesh->numVertices * 3 * sizeof(float), mesh->vertices, GL_STATIC_DRAW);
    /* - Normals */
    glBindBuffer(GL_ARRAY_BUFFER, glo->vbo[1]);
    glBufferData(GL_ARRAY_BUFFER, mesh->numVertices * 3 * sizeof(float), mesh->normals, GL_STATIC_DRAW);
    /* - TexCoords */
    glBindBuffer(GL_ARRAY_BUFFER, glo->vbo[2]);
    glBufferData(GL_ARRAY_BUFFER, mesh->numVertices * 2 * sizeof(float), mesh->texCoords, GL_STATIC_DRAW);
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
    if (glIsBuffer(glo->vbo[0]) == GL_TRUE) {
        glDeleteBuffers(1, &glo->vbo[0]);
    }
    if (glIsBuffer(glo->vbo[1]) == GL_TRUE) {
        glDeleteBuffers(1, &glo->vbo[1]);
    }
    if (glIsBuffer(glo->vbo[2]) == GL_TRUE) {
        glDeleteBuffers(1, &glo->vbo[2]);
    }
    if (glIsVertexArray(glo->vao) == GL_TRUE) {
        glDeleteVertexArrays(1, &glo->vao);
    }
}
