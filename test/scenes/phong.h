#include <stdlib.h>
#include <game/material/phong.h>
#include <game/mesh/icosphere.h>
#include <game/render/lights_buffer_object.h>
#include <game/scene/scene.h>
#include "util.h"

static int phong_setup(struct Scene* scene, int withSpecular) {
    struct PhongMaterial* mat = NULL;
    struct Node* n = NULL;
    struct Mesh mesh;
    int ok;

    if (!make_icosphere(&mesh, 2, 4)) return 0;
    ok = (mat = phong_material_new(0)) && (n = create_node(&mesh, mat));
    mesh_free(&mesh);
    ok = ok && scene_add(scene, n);
    if (!ok && n) free_node(n);
    if (ok) {
        material_param_set_vec3_elems(&mat->ambient, 0.1, 0.0, 0.1);
        material_param_set_vec3_elems(&mat->diffuse, 0.6, 0.0, 0.6);
        material_param_set_vec3_elems(&mat->specular, withSpecular ? 0.1 : 0.0, 0.0, withSpecular ? 0.1 : 0.0);
        material_param_set_float_constant(&mat->shininess, 1.0);
    } else {
        if (n) {
            free_node(n);
        } else {
            free(mat);
        }
    }
    return ok;
}

