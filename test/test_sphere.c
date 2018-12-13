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
#include "test/util/main.h"

static const struct EarthTexture {
    const char* path;
    double width, height, ratio;
} texture[NUM_SPHERE_MAP] = {
    {"textures/earth_central_cylindric.png", 702.0 / 1024.0, 702.0 / 1024.0, 1.0},
    {"textures/earth_mercator.png", 2044.0 / 2048.0, 1730.0 / 2048.0, 1.0},
    {"textures/earth_miller.png", 2044.0 / 2048.0, 1498.0 / 2048.0, 1.0},
    {"textures/earth_equirectangular.png", 2042.0 / 2048.0, 1020.0 / 1024.0, 2.0}
};

static struct GLObject sphere = {0};

int run(struct Viewer* viewer, struct Scene* scene, int argc, char** argv) {
    struct Mesh sphereMesh = {0};
    enum SphereMapType type = SPHERE_MAP_MILLER;
    int ret = 1;

    if (argc >= 1) {
        if ((type = strtoul(argv[0], NULL, 10)) < NUM_SPHERE_MAP) {
            type = SPHERE_MAP_MILLER;
        }
    }

    if (!make_icosphere(&sphereMesh, 2.0, 4)) {
        fprintf(stderr, "Error: failed to create sphere\n");
    } else if (!compute_sphere_uv(&sphereMesh, texture[type].width, texture[type].height, texture[type].ratio, type)) {
        fprintf(stderr, "Error: failed to compute UVs\n");
    } else {
        sphere.vertexArray = vertex_array_new(&sphereMesh);
        if (!(sphere.material = (struct Material*)solid_texture_material_new(asset_manager_load_texture(texture[type].path)))) {
            fprintf(stderr, "Error: failed to create geometry\n");
        } else {
            scene->root.object = &sphere;
            node_rotate(&scene->root, VEC3_AXIS_X, -M_PI / 2.0);
            viewer->wheel_callback = wheel_callback;
            viewer->cursor_callback = cursor_rotate_object;
            viewer->callbackData = &scene->root;
            ret = 0;
        }
    }

    mesh_free(&sphereMesh);

    return ret;
}
