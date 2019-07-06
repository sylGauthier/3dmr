#include <stdlib.h>
#include <game/material/solid.h>
#include <game/mesh/mesh.h>
#include <game/render/globject.h>
#include <game/scene/node.h>
#include <game/scene/frame.h>

static const float arrowVertices[] = {
    0.0,  0.1,  0.0,   1.0, 0.0, 0.0,   0.0,  0.0,  0.1,
    0.0,  0.0,  0.1,   1.0, 0.0, 0.0,   0.0, -0.1,  0.0,
    0.0, -0.1,  0.0,   1.0, 0.0, 0.0,   0.0,  0.0, -0.1,
    0.0,  0.0, -0.1,   1.0, 0.0, 0.0,   0.0,  0.1,  0.0
};

static const struct Mesh arrow = {(float*)arrowVertices, NULL, 12, 0, 0};

struct Node* make_frame(void) {
    struct GLObject* gl = NULL;
    struct Node* node;
    struct VertexArray* va;
    struct Material* material[3];
    unsigned int i;

    if (!(node = malloc(4 * sizeof(struct Node)))
     || !(gl = malloc(3 * sizeof(struct GLObject)))) {
        free(node);
        free(gl);
        return NULL;
    }

    node_init(node);
    va = vertex_array_new(&arrow);
    material[0] = (struct Material*)solid_color_material_new(1.0, 0.0, 0.0);
    material[1] = (struct Material*)solid_color_material_new(0.0, 1.0, 0.0);
    material[2] = (struct Material*)solid_color_material_new(0.0, 0.0, 1.0);
    for (i = 0; i < 3; i++) {
        gl[i].vertexArray = va;
        gl[i].material = material[i];
        node_init(node + i + 1);
        node_set_geometry(node + i + 1, gl + i);
        node_add_child(node, node + i + 1);
    }
    node_rotate(node + 2, VEC3_AXIS_Z, M_PI / 2.0);
    node_rotate(node + 3, VEC3_AXIS_Y, -M_PI / 2.0);

    return node;
}

void free_frame(struct Node* frame) {
    unsigned int i;

    for (i = 1; i < 4; i++) {
        free(frame[i].data.geometry->material);
    }
    vertex_array_free(frame[1].data.geometry->vertexArray);
    free(frame[1].data.geometry);
    nodes_free(frame, NULL);
    free(frame);
}
