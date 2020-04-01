#include <stdlib.h>
#include <game/material/solid.h>
#include <game/mesh/box.h>
#include <game/render/texture.h>
#include <game/scene/scene.h>
#include "util.h"

static int alpha_setup(enum AlphaMode alphaMode, struct AlphaParams* params) {
    GLuint alphamap;
    if (!alphaMode) return 1;
    if (!(alphamap = texture_load_from_png("assets/grad.png"))) return 0;
    material_param_set_float_texture(&params->alpha, alphamap);
    params->mode = alphaMode;
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    return 1;
}

static int solid_color_setup(struct Scene* scene, float size, enum AlphaMode alphaMode) {
    struct Node* n = NULL;
    struct Mesh mesh;
    struct SolidMaterialParams* params = NULL;
    struct Material* mat = NULL;
    GLuint alphamap;

    if (!make_box(&mesh, size, size, size)) return 0;
    if ((params = solid_material_params_new())) {
        material_param_set_vec3_elems(&params->color, 1, 0, 1);
        if (alpha_setup(alphaMode, &params->alpha)) {
            if (alphaMode) alphamap = params->alpha.alpha.value.texture;
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
            if (alphaMode) glDeleteTextures(1, &alphamap);
        }
        free(params);
    }
    mesh_free(&mesh);
    return 0;
}
