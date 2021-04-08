#include <stdlib.h>

#include <3dmr/material/pbr.h>
#include <3dmr/mesh/quad.h>
#include <3dmr/scene/scene.h>
#include <3dmr/render/lights_buffer_object.h>
#include <3dmr/render/texture.h>

#include "util.h"

#define SIZE 2.0165

void setup_lighting(struct Scene* scene) {
    struct DirectionalLight l = {{0, 0, -1}, {0.1, 0, 0}};
    struct AmbientLight a = {{0, 0, 100}};
    lights_buffer_object_update_ambient(&scene->lights, &a);
    lights_buffer_object_update_dlight(&scene->lights, &l, 0);
    lights_buffer_object_update_ndlight(&scene->lights, 1);
    uniform_buffer_send(&scene->lights);
}

int occlusionmap_setup(struct Scene* scene) {
    struct Node* n = NULL;
    struct Mesh mesh;
    struct PBRMaterialParams* params = NULL;
    struct Material* mat = NULL;
    GLuint tex;

    if (!make_quad(&mesh, SIZE, SIZE)) return 0;
    if ((params = pbr_material_params_new())) {
        if ((tex = texture_load_from_png("assets/occlusion.png"))) {
            params->occlusionMap = tex;
            if ((mat = pbr_material_new(mesh.flags, params))) {
                if ((n = create_node(&mesh, mat, params))) {
                    if (scene_add(scene, n)) {
                        setup_lighting(scene);
                        mesh_free(&mesh);
                        return 1;
                    }
                    free_node(n);
                    mat = NULL;
                    params = NULL;
                }
                free(mat);
            }
            glDeleteTextures(1, &tex);
        }
        free(params);
    }
    mesh_free(&mesh);
    return 0;
}

void occlusionmap_teardown(struct Scene* scene) {
}
