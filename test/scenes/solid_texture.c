#include <game/material/solid.h>
#include <game/mesh/box.h>
#include <game/render/texture.h>
#include <game/scene/scene.h>
#include "util.h"

#define SIZE 2.0165

int solid_texture_setup(struct Scene* scene) {
    struct Node* n;
    struct Mesh mesh;
    GLuint tex;
    int ok;

    if (!make_box(&mesh, SIZE, SIZE, SIZE)) return 0;
    if (!(tex = texture_load_from_file("assets/rgb_tux.png"))) {
        mesh_free(&mesh);
        return 0;
    }
    ok = (n = create_node(&mesh, solid_texture_material_new(tex))) != NULL;
    mesh_free(&mesh);
    ok = ok && scene_add(scene, n);
    if (!ok && n) free_node(n);
    if (!ok) glDeleteTextures(1, &tex);
    return ok;
}

void solid_texture_teardown(struct Scene* scene) {
    glDeleteTextures(1, &((struct SolidTextureMaterial*)scene->root.children[0]->data.geometry->material)->texture);
}
