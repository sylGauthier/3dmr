#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <game/asset_manager.h>
#include <game/material/solid_texture.h>
#include <game/math/linear_algebra.h>
#include <game/mesh/icosphere.h>
#include <game/mesh/sphere_uv.h>
#include <game/render/globject.h>
#include <game/render/viewer.h>
#include <game/scene/node.h>
#include <game/scene/scene.h>

#include "test/util/callbacks.h"

static const struct EarthTexture {
    const char* path;
    double width, height, ratio;
} texture[NUM_SPHERE_MAP] = {
    {"png/earth_central_cylindric.png", 702.0 / 1024.0, 702.0 / 1024.0, 1.0},
    {"png/earth_mercator.png", 2044.0 / 2048.0, 1730.0 / 2048.0, 1.0},
    {"png/earth_miller.png", 2044.0 / 2048.0, 1498.0 / 2048.0, 1.0},
    {"png/earth_equirectangular.png", 2042.0 / 2048.0, 1020.0 / 1024.0, 2.0}
};

static const enum SphereMapType type = SPHERE_MAP_MILLER;

int main(int argc, char** argv) {
    struct Viewer* viewer = NULL;
    struct Scene scene;
    struct Mesh sphereMesh = {0};
    struct GLObject sphere = {0};
    struct Geometry* geom = NULL;
    int ret = 1;

    asset_manager_add_path(".");
    asset_manager_add_path("./test/assets");
    asset_manager_add_path("..");
    asset_manager_add_path("../test/assets");

    if (!(viewer = viewer_new(640, 480, __FILE__))) {
        fprintf(stderr, "Error: cannot start viewer\n");
    } else if (!make_icosphere(&sphereMesh, 2.0, 4)) {
        fprintf(stderr, "Error: failed to create sphere\n");
    } else if (!compute_sphere_uv(&sphereMesh, texture[type].width, texture[type].height, texture[type].ratio, type)) {
        fprintf(stderr, "Error: failed to compute UVs\n");
    } else {
        sphere.vertexArray = vertex_array_new(&sphereMesh);
        if (!(sphere.material = (struct Material*)solid_texture_material_new(asset_manager_load_texture(texture[type].path)))) {
            fprintf(stderr, "Error: failed to create geometry\n");
        } else {
            scene_init(&scene);
            scene.root.object = &sphere;
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
            ret = 0;

            scene_free(&scene);
        }
    }

    free(sphere.material);
    vertex_array_free(sphere.vertexArray);
    mesh_free(&sphereMesh);
    viewer_free(viewer);

    return ret;
}
