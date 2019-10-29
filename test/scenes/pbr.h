#include <stdlib.h>
#include <game/material/pbr.h>
#include <game/mesh/icosphere.h>
#include <game/render/lights_buffer_object.h>
#include <game/render/vertex_shader.h>
#include <game/scene/scene.h>
#include "util.h"

static int pbr_setup(struct Scene* scene) {
    struct PBRMaterialParams* params = NULL;
    struct Material* mat = NULL;
    struct Node* n = NULL;
    struct Mesh mesh;
    GLuint shaders[2] = {0, 0};

    if (!make_icosphere(&mesh, 2, 4)) return 0;
    if ((params = pbr_material_params_new())) {
        material_param_set_vec3_elems(&params->albedo, 1, 0, 1);
        material_param_set_float_constant(&params->metalness, 0.0);
        material_param_set_float_constant(&params->roughness, 0.3);
        shaders[0] = vertex_shader_standard(mesh.flags);
        shaders[1] = pbr_shader_new(params);
        if (shaders[0] && shaders[1] && (mat = material_new_from_shaders(shaders, 2, pbr_load, params, GL_FILL))) {
            if ((n = create_node(&mesh, mat))) {
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
