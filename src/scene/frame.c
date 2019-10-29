#include <stdlib.h>
#include <game/init.h>
#include <game/material/solid.h>
#include <game/mesh/mesh.h>
#include <game/render/shader.h>
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
    struct Material* materials;
    struct SolidMaterialParams* matParams;
    struct Geometry* geometry;
    unsigned int i;
    GLuint shaders[2] = {0, 0}, prog = 0;

    if (!(node = malloc(4 * sizeof(*node) + sizeof(*va) + 3 * sizeof(*materials) + 3 * sizeof(*matParams) + 3 * sizeof(*geometry)))) {
        return NULL;
    }
    va = (void*)(node + 4);
    materials = (void*)(va + 1);
    matParams = (void*)(materials + 3);
    geometry = (void*)(matParams + 3);

    node_init(node);
    for (i = 0; i < 3; i++) {
        solid_material_params_init(matParams + i);
        material_param_set_vec3_elems(&matParams[i].color, i == 0, i == 1, i == 2);
    }
    shaders[0] = shader_find_compile("standard.vert", GL_VERTEX_SHADER, &shaderRootPath, 1, NULL, 0);
    shaders[1] = solid_shader_new(matParams);
    if (shaders[0] && shaders[1]) prog = shader_link(shaders, 2);
    if (shaders[0]) glDeleteShader(shaders[0]);
    if (shaders[1]) glDeleteShader(shaders[1]);
    if (!prog) {
        free(node);
        return NULL;
    }
    vertex_array_gen(&arrow, va);
    for (i = 0; i < 3; i++) {
        geometry[i].vertexArray = va;
        geometry[i].material = materials + i;
        materials[i].load = solid_load;
        materials[i].params = matParams + i;
        materials[i].polygonMode = GL_FILL;
        materials[i].program = prog;
        node_init(node + i + 1);
        node_set_geometry(node + i + 1, geometry + i);
        node_add_child(node, node + i + 1);
    }
    node_rotate(node + 2, VEC3_AXIS_Z, M_PI / 2.0);
    node_rotate(node + 3, VEC3_AXIS_Y, -M_PI / 2.0);

    return node;
}

void free_frame(struct Node* frame) {
    glDeleteProgram(frame[1].data.geometry->material->program);
    vertex_array_del(frame[1].data.geometry->vertexArray);
    nodes_free(frame, NULL);
    free(frame);
}
