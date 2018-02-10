#include "scenes.h"
#include "linear_algebra.h"
#include "mesh/mesh.h"
#include "mesh/box.h"
#include <stdlib.h>

void new_geom_nxn(struct Geometry *geom, int len, float space, struct Node *root) {
    struct Node *nodes, *n;
    Vec3 offset = {0};
    int i, x, z;

    nodes  = malloc((len * len) * sizeof(struct Node));

    i = 0;
    for (x = 0; x < len; x++) {
        offset[0] = x * space;
        for (z = 0; z < len; z++, i++) {
            n = &nodes[i];
            node_init(n, geom);
            node_add_child(root, n);
            offset[2] = z * space;
            node_translate(n, offset);
        }
    }
}

struct Node *box_10x10(struct Geometry *mat, float size, float spacing) {
    struct Node *root;
    struct Mesh mesh;
    struct Geometry *g;
    Vec3 offset;

    root = malloc(sizeof(struct Node));
    g = malloc(sizeof(struct Geometry));

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

