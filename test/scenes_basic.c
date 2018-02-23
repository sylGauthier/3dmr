#include <stdlib.h>
#include "test/scenes_util.h"
#include "test/scenes_basic.h"
#include "mesh/box.h"
#include "mesh/icosphere.h"
#include "linear_algebra.h"

struct Node *box_surface(struct Geometry* mat, float size, int slen, float spacing) {
    struct Node *root;
    struct Mesh mesh;
    struct Geometry *g;

    root = malloc(sizeof(struct Node));
    g = malloc(sizeof(struct Geometry));
    if (!root || !g) {
        free(root);
        free(g);
        return NULL;
    }

    *g = *mat;

    make_box(&mesh, size, size, size);
    globject_new(&mesh, &g->glObject);
    mesh_free(&mesh);

    node_init(root, NULL);

    new_geom_surface(g, slen, spacing, root);
    translate_to_center(slen, spacing, root);

    return root;
}

struct Node *sphere_surface(struct Geometry* mat, float radius, int slen, float spacing) {
    struct Node *root;
    struct Mesh mesh;
    struct Geometry *g;

    root = malloc(sizeof(struct Node));
    g = malloc(sizeof(struct Geometry));
    if (!root || !g) {
        free(root);
        free(g);
        return NULL;
    }

    *g = *mat;

    make_icosphere(&mesh, radius, 2);
    globject_new(&mesh, &g->glObject);
    mesh_free(&mesh);

    node_init(root, NULL);

    new_geom_surface(g, slen, spacing, root);
    translate_to_center(slen, spacing, root);

    return root;
}

void spheres_and_boxes(struct Geometry* smat, struct Geometry* bmat, struct Node* root) {
    struct Node *spheres, *boxes;

    if ((boxes = box_surface(bmat, 1, 10, 2))) {
        node_add_child(root, boxes);
    }

    if ((spheres = sphere_surface(smat, 0.5, 11, 2))) {
        node_add_child(root, spheres);
    }
}
