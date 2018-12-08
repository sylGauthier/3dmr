#include <stdlib.h>
#include <game/mesh/box.h>
#include <game/mesh/icosphere.h>
#include <game/render/vertex_array.h>
#include "scenes_util.h"
#include "scenes_basic.h"

struct Node* box_surface(struct Material* mat, float size, int slen, float spacing) {
    struct Node* root;
    struct Mesh mesh;
    struct GLObject* o;

    root = malloc(sizeof(struct Node));
    o = malloc(sizeof(struct GLObject));
    if (!root || !o) {
        free(root);
        free(o);
        return NULL;
    }

    o->material = mat;

    make_box(&mesh, size, size, size);
    o->vertexArray = vertex_array_new(&mesh);
    mesh_free(&mesh);

    node_init(root, NULL);

    new_geom_surface(o, slen, spacing, root);
    translate_to_center(slen, spacing, root);

    return root;
}

struct Node* sphere_surface(struct Material* mat, float radius, int slen, float spacing) {
    struct Node* root;
    struct Mesh mesh;
    struct GLObject* o;

    root = malloc(sizeof(struct Node));
    o = malloc(sizeof(struct GLObject));
    if (!root || !o) {
        free(root);
        free(o);
        return NULL;
    }

    o->material = mat;

    make_icosphere(&mesh, radius, 2);
    o->vertexArray = vertex_array_new(&mesh);
    mesh_free(&mesh);

    node_init(root, NULL);

    new_geom_surface(o, slen, spacing, root);
    translate_to_center(slen, spacing, root);

    return root;
}

void spheres_and_boxes(struct Material* smat, struct Material* bmat, struct Node* root) {
    struct Node *spheres, *boxes;

    if ((boxes = box_surface(bmat, 1, 10, 2))) {
        node_add_child(root, boxes);
    }

    if ((spheres = sphere_surface(smat, 0.5, 11, 2))) {
        node_add_child(root, spheres);
    }
}
