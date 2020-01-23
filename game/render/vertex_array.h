#include <GL/glew.h>
#include <game/mesh/mesh.h>

#ifndef VERTEX_ARRAY_H
#define VERTEX_ARRAY_H

struct VertexArray {
    GLuint vbo, ibo, vao;
    unsigned int numVertices, numIndices, flags;
    Vec3 boundingBox[2];
};

void vertex_array_gen(const struct Mesh* mesh, struct VertexArray* va);
void vertex_array_del(struct VertexArray* va);

struct VertexArray* vertex_array_new(const struct Mesh* mesh);
void vertex_array_free(struct VertexArray* va);

void vertex_array_render(const struct VertexArray* vertexArray);

#endif
