#include <stdlib.h>
#include <game/material/solid.h>
#include <game/mesh/box.h>
#include <game/scene/scene.h>
#include "util.h"

static int solid_color_setup(struct Scene* scene, float size) {
    struct Node* n = NULL;
    struct Mesh mesh;
    struct SolidMaterial* mat = NULL;
    int ok;

    if (!make_box(&mesh, size, size, size)) return 0;
    ok = (mat = solid_material_new(MAT_PARAM_CONSTANT)) && (n = create_node(&mesh, mat));
    mesh_free(&mesh);
    if (ok) material_param_set_vec3_elems(&mat->color, 1, 0, 1);
    ok = ok && scene_add(scene, n);
    if (!ok) {
        if (n) {
            free_node(n);
        } else {
            free(mat);
        }
    }
    return ok;
}

