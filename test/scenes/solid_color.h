#include <game/material/solid.h>
#include <game/mesh/box.h>
#include <game/scene/scene.h>
#include "util.h"

static int solid_color_setup(struct Scene* scene, float size) {
    struct Node* n;
    struct Mesh mesh;
    int ok;

    if (!make_box(&mesh, size, size, size)) return 0;
    ok = (n = create_node(&mesh, solid_color_material_new(1, 0, 1))) != NULL;
    mesh_free(&mesh);
    ok = ok && scene_add(scene, n);
    if (!ok && n) free_node(n);
    return ok;
}

