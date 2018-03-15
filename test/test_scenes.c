#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <unistd.h>

#include "viewer.h"
#include "camera.h"
#include "globject.h"
#include "texture.h"
#include "scene.h"
#include "asset_manager.h"
#include "geometry/phong_color.h"
#include "geometry/phong_texture.h"
#include "test/util/scenes_basic.h"
#include "test/util/color.h"
#include "test/util/light.h"
#include "test/util/checkerboard.h"

int running;

static void key_callback(struct Viewer* viewer, int key, int scancode, int action, int mods, void* userData) {
    switch (key) {
        case GLFW_KEY_ESCAPE:
            running = 0;
            break;
        case GLFW_KEY_F12:
            viewer_screenshot(viewer, "test_scene_screenshot.png");
            break;
    }
}

static void close_callback(struct Viewer* viewer, void* userData) {
    running = 0;
}

int run() {
    struct Viewer* viewer;
    struct Scene scene;
    struct GLObject bloat = {0};
    struct Geometry *mat_checkerboard;
    struct Geometry *mat_solidcolor;
    struct PhongMaterial phong = {
        {1.0, 1.0, 1.0},
        {1.0, 1.0, 1.0},
        {1.0, 1.0, 1.0},
        1.0
    };

    asset_manager_add_path(".");
    asset_manager_add_path("..");
    asset_manager_add_path("../..");
    viewer = viewer_new(1024, 768, "test_scenes");
    viewer->key_callback = key_callback;
    viewer->close_callback = close_callback;
    running = 1;

    mat_checkerboard = phong_texture_geometry(&bloat, default_checkerboard(), &phong);
    mat_solidcolor = phong_color_geometry(&bloat, 0, 1.0, 0, &phong);

    scene_init(&scene);
    scene.lights.numLocal = 1;
    test_init_local_light(&scene.lights.local[0]);

    spheres_and_boxes(mat_solidcolor, mat_checkerboard, &scene.root);
    node_rotate(&scene.root, (void *)VEC3_AXIS_X, M_PI / 2.0);

    viewer->callbackData = &scene.root;

    while (running) {
        viewer_process_events(viewer);
        usleep(10 * 1000);
        viewer_next_frame(viewer);
        scene_render(&scene, &viewer->camera);
    }

    free(mat_checkerboard);
    free(mat_solidcolor);
    viewer_free(viewer);
    scene_free(&scene);

    return EXIT_SUCCESS;
}

void usage() {
    puts("Usage: test_scenes");
    puts("Takes no options");
}

int main(int argc, char *argv[]) {
    if (argc > 1) {
	usage();
	return EXIT_FAILURE;
    }
    return run();
}
