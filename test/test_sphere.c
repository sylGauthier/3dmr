#include <stdio.h>
#include <stdlib.h>
#include "viewer.h"
#include "scene.h"
#include "node.h"
#include "asset_manager.h"
#include "globject.h"
#include "linear_algebra.h"
#include "mesh/icosphere.h"
#include "mesh/sphere_uv.h"
#include "geometry/solid_texture.h"
#include "test/util/callbacks.h"

static const struct EarthTexture {
    const char* path;
    double width, height, ratio;
} texture[NUM_SPHERE_MAP] = {
    {"png/earth_central_cylindric.png", 702.0 / 1024.0, 702.0 / 1024.0, 1.0},
    {"png/earth_mercator.png", 2044.0 / 2048.0, 1730.0 / 2048.0, 1.0},
    {"png/earth_miller.png", 1016.0 / 1024.0, 745.0 / 1024.0, 1.0},
    {"png/earth_equirectangular.png", 2042.0 / 2048.0, 1020.0 / 1024.0, 2.0}
};

static const enum SphereMapType type = SPHERE_MAP_MILLER;

int main(int argc, char** argv) {
    struct Viewer* viewer = NULL;
    struct Scene scene;
    struct Mesh sphere = {0};
    struct GLObject sphereGl = {0};
    struct Geometry* geom = NULL;
    int ret = 1;

    asset_manager_add_path(".");
    asset_manager_add_path("./test/assets");
    asset_manager_add_path("..");
    asset_manager_add_path("../test/assets");

    if (!(viewer = viewer_new(640, 480, __FILE__))) {
        fprintf(stderr, "Error: cannot start viewer\n");
    } else if (!make_icosphere(&sphere, 2.0, 4)) {
        fprintf(stderr, "Error: failed to create sphere\n");
    } else if (!compute_sphere_uv(&sphere, texture[type].width, texture[type].height, texture[type].ratio, type)) {
        fprintf(stderr, "Error: failed to compute UVs\n");
    } else {
        globject_new(&sphere, &sphereGl);
        if (!(geom = solid_texture_geometry(&sphereGl, asset_manager_load_texture(texture[type].path)))) {
            fprintf(stderr, "Error: failed to create geometry\n");
        } else {
            scene_init(&scene);
            scene.root.geometry = geom;
            node_rotate(&scene.root, VEC3_AXIS_X, -M_PI / 2.0);
            viewer->close_callback = close_callback;
            viewer->wheel_callback = wheel_callback;
            viewer->cursor_callback = cursor_rotate_object;
            viewer->callbackData = &scene.root;

            running = 1;
            while (running) {
                usleep(10 * 1000);
                viewer_process_events(viewer);
                viewer_next_frame(viewer);
                scene_render(&scene, &viewer->camera);
            }

            scene_free(&scene);
        }
    }

    free(geom);
    globject_free(&sphereGl);
    mesh_free(&sphere);
    viewer_free(viewer);

    return ret;
}
