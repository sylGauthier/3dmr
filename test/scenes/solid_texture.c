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
    struct SolidMaterial* mat = NULL;
    GLuint tex;
    int ok;

    if (!make_box(&mesh, SIZE, SIZE, SIZE)) return 0;
    if (!(tex = texture_load_from_png("assets/rgb_tux.png"))) {
        mesh_free(&mesh);
        return 0;
    }
    ok = (mat = solid_material_new(MAT_PARAM_TEXTURED)) && (n = create_node(&mesh, mat));
    mesh_free(&mesh);
    if (ok) material_param_set_vec3_texture(&mat->color, tex);
    ok = ok && scene_add(scene, n);
    if (!ok) {
        if (n) {
            free_node(n);
        } else {
            free(mat);
        }
        glDeleteTextures(1, &tex);
    }
    return ok;
}

void solid_texture_teardown(struct Scene* scene) {
    glDeleteTextures(1, &((struct SolidMaterial*)scene->root.children[0]->data.geometry->material)->color.value.texture);
}
