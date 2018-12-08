#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <game/asset_manager.h>
#include <game/material/solid_color.h>
#include <game/mesh/box.h>
#include <game/render/globject.h>
#include <game/render/viewer.h>
#include <game/scene/scene.h>

int main(int argc, char** argv) {
    struct Viewer* viewer = NULL;
    struct Scene scene;
    struct Mesh cubeMesh = {0};
    struct GLObject cube = {0};
    double size;
    int ret = 1;

    asset_manager_add_path(".");
    asset_manager_add_path("..");

    if (argc != 3) {
        fprintf(stderr, "Usage: %s size out.png\n", argv[0]);
    } else if (!(size = strtod(argv[1], NULL))) {
        fprintf(stderr, "Error: bad size\n");
    } else if (!(viewer = viewer_new(640, 480, __FILE__))) {
        fprintf(stderr, "Error: cannot start viewer\n");
    } else if (!make_box(&cubeMesh, size, size, size)) {
        fprintf(stderr, "Error: failed to create cube\n");
    } else {
        cube.vertexArray = vertex_array_new(&cubeMesh);
        if (!(cube.material = (struct Material*)solid_color_material_new(1.0, 0.0, 1.0))) {
            fprintf(stderr, "Error: failed to create geometry\n");
        } else {
            scene_init(&scene);
            scene.root.object = &cube;

            viewer_next_frame(viewer);
            scene_render(&scene, &viewer->camera);
            usleep(1000000);

            if (viewer_screenshot(viewer, argv[2])) {
                ret = 0;
            } else {
                fprintf(stderr, "Error: failed to take screenshot\n");
            }
            scene_free(&scene);
        }
    }

    free(cube.material);
    vertex_array_free(cube.vertexArray);
    mesh_free(&cubeMesh);
    viewer_free(viewer);

    return ret;
}
