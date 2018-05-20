#include <stdlib.h>
#include "frame.h"
#include "globject.h"
#include "node.h"
#include "geometry/solid_color.h"
#include "mesh/mesh.h"

static const float arrowVertices[] = {
    0.0,  0.1,  0.0,   1.0, 0.0, 0.0,   0.0,  0.0,  0.1,
    0.0,  0.0,  0.1,   1.0, 0.0, 0.0,   0.0, -0.1,  0.0,
    0.0, -0.1,  0.0,   1.0, 0.0, 0.0,   0.0,  0.0, -0.1,
    0.0,  0.0, -0.1,   1.0, 0.0, 0.0,   0.0,  0.1,  0.0
};

static const struct Mesh arrow = {(float*)arrowVertices, NULL, NULL, NULL, 12, 0, 0, 0};

struct Node* make_frame(void) {
    struct GLObject gl;
    struct Node* node;

    if (!(node = malloc(4 * sizeof(struct Node)))) {
        return NULL;
    }

    globject_new(&arrow, &gl);
    node_init(node, NULL);
    node_init(node + 1, solid_color_geometry(&gl, 1.0, 0.0, 0.0));
    node_init(node + 2, solid_color_geometry(&gl, 0.0, 1.0, 0.0));
    node_init(node + 3, solid_color_geometry(&gl, 0.0, 0.0, 1.0));
    node_add_child(node, node + 1);
    node_add_child(node, node + 2);
    node_add_child(node, node + 3);
    node_rotate(node + 2, (float*)VEC3_AXIS_Z, M_PI / 2.0);
    node_rotate(node + 3, (float*)VEC3_AXIS_Y, -M_PI / 2.0);

    return node;
}

void free_frame(struct Node* frame) {
    globject_free(&frame[1].geometry->glObject);
    graph_free(frame);
    free(frame);
}
