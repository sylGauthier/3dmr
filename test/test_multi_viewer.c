#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <unistd.h>

#include <game/asset_manager.h>
#include <game/material/phong_color.h>
#include <game/material/phong_texture.h>
#include <game/material/solid_color.h>
#include <game/material/solid_texture.h>
#include <game/mesh/box.h>
#include <game/render/texture.h>
#include <game/render/viewer.h>
#include <game/scene/scene.h>

#include "test/util/scenes_basic.h"
#include "test/util/color.h"
#include "test/util/light.h"
#include "test/util/callbacks.h"

int main() {
    struct Viewer *viewer, *viewer2;
    double t = 0.0, dt;
    struct Mesh cubeMesh;
    struct GLObject cube = {0};
    struct Scene scene, scene2;
    struct Node nodeCube;
    struct PhongMaterial phongMat = {
        {1.0, 1.0, 1.0},
        {1.0, 1.0, 1.0},
        {1.0, 1.0, 1.0},
        1.0
    };
    struct PhongColorMaterial* sphereMat;
    struct PhongTextureMaterial* phongCubeMat;
    struct SolidTextureMaterial* solidCubeMat;

    asset_manager_add_path(".");
    asset_manager_add_path("./test/assets");
    asset_manager_add_path("..");
    asset_manager_add_path("../test/assets");
    viewer = viewer_new(640, 480, "Game");
    viewer->cursor_callback = cursor_rotate_object;
    viewer->wheel_callback = wheel_callback;
    viewer->key_callback = key_callback;
    viewer->close_callback = close_callback;
    viewer2 = viewer_new(640, 480, "Game 2");
    viewer2->cursor_callback = cursor_rotate_object;
    viewer2->wheel_callback = wheel_callback;
    viewer2->key_callback = key_callback;
    viewer2->close_callback = close_callback;
    running = 1;

    make_box(&cubeMesh, 2.0, 2.0, 2.0);
    viewer_make_current(viewer2);
    cube.vertexArray = vertex_array_new(&cubeMesh);
    mesh_free(&cubeMesh);

    viewer_make_current(viewer);
    sphereMat = phong_color_material_new(0, 0, 0, &phongMat);
    phongCubeMat = phong_texture_material_new(asset_manager_load_texture("png/rgb_tux.png"), &phongMat);
    viewer_make_current(viewer2);
    solidCubeMat = solid_texture_material_new(asset_manager_load_texture("png/rgb_tux.png"));
    cube.material = (struct Material*)solidCubeMat;

    viewer_make_current(viewer);
    scene_init(&scene);
    spheres_and_boxes((struct Material*)sphereMat, (struct Material*)phongCubeMat, &scene.root);

    test_init_point_light(&scene.lights.point[0]);
    scene.lights.numPointLights = 1;

    viewer_make_current(viewer2);
    scene_init(&scene2);
    node_init(&nodeCube, &cube);
    node_add_child(&scene2.root, &nodeCube);

    viewer->callbackData = &scene.root;
    viewer2->callbackData = &scene2.root;

    while (running) {
        viewer_process_events(viewer);
        viewer_process_events(viewer2);
        usleep(10 * 1000);

        viewer_make_current(viewer);
        dt = viewer_next_frame(viewer);
        t += 50.0 * dt;
        hsv2rgb(fmod(t, 360.0), 1.0, 1.0, sphereMat->color);
        mul3sv(scene.lights.point[0].ambient, 0.1, sphereMat->color);
        mul3sv(scene.lights.point[0].diffuse, 1.0, sphereMat->color);
        scene_render(&scene, &viewer->camera);

        viewer_make_current(viewer2);
        viewer_next_frame(viewer2);
        scene_render(&scene2, &viewer2->camera);
    }

    free(sphereMat);
    free(phongCubeMat);
    free(solidCubeMat);
    vertex_array_free(cube.vertexArray);
    viewer_free(viewer);
    viewer_free(viewer2);
    scene_free(&scene);
    scene_free(&scene2);

    return 0;
}
