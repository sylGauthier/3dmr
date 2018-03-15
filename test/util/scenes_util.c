#include <stdlib.h>
#include "scenes_util.h"
#include "linear_algebra.h"
#include "mesh/mesh.h"

void new_geom_surface(struct Geometry* geom, int slen, float spacing, struct Node* root) {
    struct Node *nodes, *n;
    Vec3 offset = {0, 0, 0};
    int i, x, z;

    if (!(nodes = malloc((slen * slen) * sizeof(struct Node)))) {
        return;
    }

    i = 0;
    for (x = 0; x < slen; x++) {
        offset[0] = x * spacing;
        for (z = 0; z < slen; z++, i++) {
            n = &nodes[i];
            node_init(n, geom);
            node_add_child(root, n);
            offset[2] = z * spacing;
            node_translate(n, &offset);
        }
    }
}

void translate_to_center(const int slen, const float spacing, struct Node* node) {
    Vec3 offset;

    offset[0] = -spacing * (slen-1) / 2.0;
    offset[1] = 0.0;
    offset[2] = -spacing * (slen-1) / 2.0;

    node_translate(node, &offset);
}
