#include <stdlib.h>
#include <game/material/solid.h>
#include <game/mesh/box.h>
#include <game/render/texture.h>
#include <game/scene/scene.h>
#include "util.h"

#define SIZE 2.0165

int solid_texture_setup(struct Scene* scene) {
    struct Node* n = NULL;
    struct Mesh mesh;
    struct SolidMaterialParams* params = NULL;
    struct Material* mat = NULL;
    GLuint tex;

    if (!make_box(&mesh, SIZE, SIZE, SIZE)) return 0;
    if ((params = solid_material_params_new())) {
        if ((tex = texture_load_from_png("assets/rgb_tux.png"))) {
            material_param_set_vec3_texture(&params->color, tex);
            if ((mat = solid_material_new(mesh.flags, params))) {
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
            glDeleteTextures(1, &tex);
        }
        free(params);
    }
    mesh_free(&mesh);
    return 0;
}

void solid_texture_teardown(struct Scene* scene) {
    glDeleteTextures(1, &((struct SolidMaterialParams*)scene->root.children[0]->data.geometry->material->params)->color.value.texture);
}
