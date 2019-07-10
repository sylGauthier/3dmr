#include <game/material/phong.h>
#include <game/mesh/icosphere.h>
#include <game/render/lights_buffer_object.h>
#include <game/scene/scene.h>
#include "util.h"

static int phong_setup(struct Scene* scene, int specular) {
    struct PhongMaterial p = {
        {0.1, 0.1, 0.1},
        {0.6, 0.6, 0.6},
        {0.1, 0.1, 0.1},
        1.0
    };
    struct Node* n;
    struct Mesh mesh;
    int ok;

    if (!specular) zero3v(p.specular);
    if (!make_icosphere(&mesh, 2, 4)) return 0;
    ok = (n = create_node(&mesh, phong_color_material_new(1, 0, 1, &p))) != NULL;
    mesh_free(&mesh);
    ok = ok && scene_add(scene, n);
    if (!ok && n) free_node(n);
    return ok;
}

