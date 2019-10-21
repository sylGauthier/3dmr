#include <stdlib.h>
#include <game/material/pbr.h>
#include <game/mesh/icosphere.h>
#include <game/render/lights_buffer_object.h>
#include <game/scene/scene.h>
#include "util.h"

static int pbr_setup(struct Scene* scene) {
    struct PBRMaterial* mat = NULL;
    struct Node* n = NULL;
    struct Mesh mesh;
    int ok;

    if (!make_icosphere(&mesh, 2, 4)) return 0;
    ok = (mat = pbr_material_new(0)) && (n = create_node(&mesh, mat));
    mesh_free(&mesh);
    ok = ok && scene_add(scene, n);
    if (!ok && n) free_node(n);
    if (ok) {
        material_param_set_vec3_elems(&mat->albedo, 1, 0, 1);
        material_param_set_float_constant(&mat->metalness, 0.0);
        material_param_set_float_constant(&mat->roughness, 0.3);
    } else {
        if (n) {
            free_node(n);
        } else {
            free(mat);
        }
    }
    return ok;
}
