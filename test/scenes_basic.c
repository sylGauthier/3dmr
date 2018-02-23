#include <stdlib.h>
#include "test/scenes_util.h"
#include "test/scenes_basic.h"
#include "mesh/box.h"
#include "mesh/icosphere.h"
#include "linear_algebra.h"

struct Node *box_10x10(struct Geometry* mat, float size, float spacing) {
    struct Node *root;
    struct Mesh mesh;
    struct Geometry *g;
    Vec3 offset;

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

    new_geom_nxn(g, 10, spacing, root);

    offset[0] = -spacing * 5.5;
    offset[2] = -spacing * 5.5;
    node_translate(root, offset);

    return root;
}

struct Node *sphere_10x10(struct Geometry* mat, float radius, float spacing) {
    struct Node *root;
    struct Mesh mesh;
    struct Geometry *g;
    Vec3 offset;

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

    new_geom_nxn(g, 10, spacing, root);

    offset[0] = -spacing * 5.5;
    offset[2] = -spacing * 5.5;
    node_translate(root, offset);

    return root;
}

void spheres_and_boxes(struct Geometry* smat, struct Geometry* bmat, struct Node* root) {
    struct Node *spheres, *boxes;
    Vec3 t = {0};

    if ((boxes = box_10x10(bmat, 1, 2))) {
        node_add_child(root, boxes);
    }

    if ((spheres = sphere_10x10(smat, 0.5, 2))) {
        t[0] = 2 * 0.5;
        t[2] = 2 * 0.5;
        node_translate(spheres, t);
        node_add_child(root, spheres);
    }
}
