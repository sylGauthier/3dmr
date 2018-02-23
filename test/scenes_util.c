#include <stdlib.h>
#include "scenes_util.h"
#include "linear_algebra.h"
#include "mesh/mesh.h"

void new_geom_nxn(struct Geometry* geom, int len, float space, struct Node* root) {
    struct Node *nodes, *n;
    Vec3 offset = {0, 0, 0};
    int i, x, z;

    if (!(nodes = malloc((len * len) * sizeof(struct Node)))) {
        return;
    }

    i = 0;
    for (x = 0; x < len; x++) {
        offset[0] = x * space;
        for (z = 0; z < len; z++, i++) {
            n = &nodes[i];
            node_init(n, geom);
            node_add_child(root, n);
            offset[2] = z * space;
            node_translate(n, &offset);
        }
    }
}

