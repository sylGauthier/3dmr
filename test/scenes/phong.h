#include <stdlib.h>
#include <3dmr/material/phong.h>
#include <3dmr/mesh/icosphere.h>
#include <3dmr/render/lights_buffer_object.h>
#include <3dmr/scene/scene.h>
#include "util.h"

static int phong_setup(struct Scene* scene, int withSpecular) {
    struct PhongMaterialParams* params = NULL;
    struct Material* mat = NULL;
    struct Node* n = NULL;
    struct Mesh mesh;

    if (!make_icosphere(&mesh, 2, 4)) return 0;
    if ((params = phong_material_params_new())) {
        material_param_set_vec3_elems(&params->ambient, 0.1, 0.0, 0.1);
        material_param_set_vec3_elems(&params->diffuse, 0.6, 0.0, 0.6);
        material_param_set_vec3_elems(&params->specular, withSpecular ? 0.1 : 0.0, 0.0, withSpecular ? 0.1 : 0.0);
        material_param_set_float_constant(&params->shininess, 1.0);
        if ((mat = phong_material_new(mesh.flags, params))) {
            if ((n = create_node(&mesh, mat, params))) {
                if (scene_add(scene, n)) {
                    mesh_free(&mesh);
                    return 1;
                }
                free_node(n);
                mat = NULL;
                params = NULL;
            }
            free(mat);
        }
        free(params);
    }
    mesh_free(&mesh);
    return 0;
}
