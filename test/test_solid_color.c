#include <stdio.h>
#include <stdlib.h>
#include "viewer.h"
#include "scene.h"
#include "asset_manager.h"
#include "globject.h"
#include "mesh/box.h"
#include "geometry/solid_color.h"

int main(int argc, char** argv) {
    struct Viewer* viewer = NULL;
    struct Scene scene;
    struct Mesh cube = {0};
    struct GLObject cubeGl = {0};
    struct Geometry* geom = NULL;
    int ret = 1;

    asset_manager_add_path(".");
    asset_manager_add_path("..");

    if (argc != 2) {
        fprintf(stderr, "Usage: %s out.png\n", argv[0]);
    } else if (!(viewer = viewer_new(640, 480, __FILE__))) {
        fprintf(stderr, "Error: cannot start viewer\n");
    } else if (!make_box(&cube, 2.0, 2.0, 2.0)) {
        fprintf(stderr, "Error: failed to create cube\n");
    } else {
        globject_new(&cube, &cubeGl);
        if (!(geom = solid_color_geometry(&cubeGl, 1.0, 0.0, 1.0))) {
            fprintf(stderr, "Error: failed to create geometry\n");
        } else {
            scene_init(&scene);
            scene.root.geometry = geom;

            viewer_next_frame(viewer);
            scene_render(&scene, &viewer->camera);

            if (viewer_screenshot(viewer, argv[1])) {
                ret = 0;
            } else {
                fprintf(stderr, "Error: failed to take screenshot\n");
            }
            scene_free(&scene);
        }
    }

    free(geom);
    globject_free(&cubeGl);
    mesh_free(&cube);
    viewer_free(viewer);

    return ret;
}
