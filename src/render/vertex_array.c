#include <stdlib.h>
#include <float.h>
#include <game/render/shader.h>
#include <game/render/vertex_array.h>

static void gen_bounding_box(const struct Mesh* mesh, struct VertexArray* va) {
    float min[3] = {FLT_MAX, FLT_MAX, FLT_MAX};
    float max[3] = {-FLT_MAX, -FLT_MAX, -FLT_MAX};
    int i, stride = MESH_FLOATS_PER_VERTEX(mesh);

    for (i = 0; i < mesh->numVertices*stride; i += stride) {
        if (mesh->vertices[i] < min[0]) {
            min[0] = mesh->vertices[i];
        }
        if (mesh->vertices[i+1] < min[1]) {
            min[1] = mesh->vertices[i+1];
        }
        if (mesh->vertices[i+2] < min[2]) {
            min[2] = mesh->vertices[i+2];
        }
        if (mesh->vertices[i] > max[0]) {
            max[0] = mesh->vertices[i];
        }
        if (mesh->vertices[i+1] > max[1]) {
            max[1] = mesh->vertices[i+1];
        }
        if (mesh->vertices[i+2] > max[2]) {
            max[2] = mesh->vertices[i+2];
        }
    }
    va->bbCenter[0] = (min[0] + max[0])/2.;
    va->bbCenter[1] = (min[1] + max[1])/2.;
    va->bbCenter[2] = (min[2] + max[2])/2.;
    va->bbDims[0] = max[0]-min[0];
    va->bbDims[1] = max[1]-min[1];
    va->bbDims[2] = max[2]-min[2];
}

void vertex_array_gen(const struct Mesh* mesh, struct VertexArray* va) {
    GLsizei stride = MESH_SIZEOF_VERTEX(mesh);
    float* offset = 0;
    unsigned int hasIndices = !!mesh->numIndices;

    va->numVertices = mesh->numVertices;
    va->numIndices = mesh->numIndices;
    gen_bounding_box(mesh, va);

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
    glVertexAttribPointer(LOCATION_VERTEX, 3, GL_FLOAT, GL_FALSE, stride, offset);
    offset += 3;
    /* - Normals */
    if (MESH_HAS_NORMALS(mesh)) {
        glEnableVertexAttribArray(LOCATION_NORMAL);
        glVertexAttribPointer(LOCATION_NORMAL, 3, GL_FLOAT, GL_FALSE, stride, offset);
        offset += 3;
    }
    /* - TexCoords */
    if (MESH_HAS_TEXCOORDS(mesh)) {
        glEnableVertexAttribArray(LOCATION_TEXCOORD);
        glVertexAttribPointer(LOCATION_TEXCOORD, 2, GL_FLOAT, GL_FALSE, stride, offset);
        offset += 2;
    }
    /* - Tangents and bitangents */
    if (MESH_HAS_TANGENTS(mesh)) {
        glEnableVertexAttribArray(LOCATION_TANGENT);
        glVertexAttribPointer(LOCATION_TANGENT, 3, GL_FLOAT, GL_FALSE, stride, offset);
        offset += 3;
        glEnableVertexAttribArray(LOCATION_BITANGENT);
        glVertexAttribPointer(LOCATION_BITANGENT, 3, GL_FLOAT, GL_FALSE, stride, offset);
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
