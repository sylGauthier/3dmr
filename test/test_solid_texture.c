#include <stdio.h>
#include <stdlib.h>

#include <game/asset_manager.h>
#include <game/material/solid_texture.h>
#include <game/mesh/box.h>
#include <game/render/globject.h>
#include <game/render/viewer.h>
#include <game/scene/scene.h>
#include "test/util/main.h"

static struct GLObject cube = {0};

int run(struct Viewer* viewer, struct Scene* scene, int argc, char** argv) {
    struct Mesh cubeMesh = {0};
    double size;
    int ret = 1;

    if (argc != 1) {
        usage("test_solid_texture", "size");
    } else if (!(size = strtod(argv[0], NULL))) {
        fprintf(stderr, "Error: bad size\n");
    } else if (!make_box(&cubeMesh, size, size, size)) {
        fprintf(stderr, "Error: failed to create cube\n");
    } else {
        cube.vertexArray = vertex_array_new(&cubeMesh);
        if (!(cube.material = (struct Material*)solid_texture_material_new(asset_manager_load_texture("png/rgb_tux.png")))) {
            fprintf(stderr, "Error: failed to create geometry\n");
        } else {
            scene->root.object = &cube;

            viewer_next_frame(viewer);
            scene_render(scene, &viewer->camera);

            ret = 0;
        }
    }

    mesh_free(&cubeMesh);

    return ret;
}
