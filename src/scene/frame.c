#include <stdlib.h>
#include <game/material/solid.h>
#include <game/mesh/mesh.h>
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
    struct Node* node;
    struct VertexArray* va;
    struct Material* material[3];
    struct SolidMaterialParams* matParams;
    struct Geometry* geometry;
    unsigned int i;

    if (!(node = malloc(4 * sizeof(*node) + sizeof(*va) + 3 * sizeof(*matParams) + 3 * sizeof(*geometry)))) {
        return NULL;
    }
    va = (void*)(node + 4);
    matParams = (void*)(va + 1);
    geometry = (void*)(matParams + 3);

    node_init(node);
    vertex_array_gen(&arrow, va);
    for (i = 0; i < 3; i++) {
        solid_material_params_init(matParams + i);
        material_param_set_vec3_elems(&matParams[i].color, i == 0, i == 1, i == 2);
        material[i] = solid_material_new(matParams + i);
    }

    if (!material[0] || !material[1] || !material[2]) {
        vertex_array_del(va);
        for (i = 0; i < 3; i++) free(material[i]);
        free(node);
        return NULL;
    }
    for (i = 0; i < 3; i++) {
        geometry[i].vertexArray = va;
        geometry[i].material = material[i];
        node_init(node + i + 1);
        node_set_geometry(node + i + 1, geometry + i);
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
    vertex_array_del(frame[1].data.geometry->vertexArray);
    nodes_free(frame, NULL);
    free(frame);
}
