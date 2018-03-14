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
#include "geometry/solid_color.h"
#include "geometry/solid_texture.h"
#include "geometry/phong_color.h"
#include "geometry/phong_texture.h"
#include "mesh/obj.h"
#include "mesh/box.h"
#include "mesh/icosphere.h"
#include "test/scenes_basic.h"
#include "test/color_util.h"
#include "test/light_util.h"

int running;

static void cursor_rotate_object(struct Viewer* viewer, double xpos, double ypos, double dx, double dy, int buttonLeft, int buttonMiddle, int buttonRight, void* data) {
    Vec3 x = {0, 1, 0}, y = {1, 0, 0};

    if (buttonLeft) {
        node_rotate(data, x, 4.0 * dx / viewer->width);
        node_rotate(data, y, 4.0 * dy / viewer->height);
    }
}

static void cursor_rotate_camera(struct Viewer* viewer, double xpos, double ypos, double dx, double dy, int buttonLeft, int buttonMiddle, int buttonRight, void* data) {
    Vec3 axis = {0, 1, 0};

    if (buttonLeft) {
        camera_rotate(&viewer->camera, axis, dx / viewer->width);
        camera_get_right(&viewer->camera, axis);
        camera_rotate(&viewer->camera, axis, dy / viewer->height);
    }
}

static void wheel_callback(struct Viewer* viewer, double xoffset, double yoffset, void* userData) {
    Vec3 axis;

    camera_get_backward(&viewer->camera, axis);
    scale3v(axis, -yoffset);
    camera_move(&viewer->camera, axis);
}

static void key_callback(struct Viewer* viewer, int key, int scancode, int action, int mods, void* userData) {
    Vec3 axis = {0, 1, 0};

    switch (key) {
        case GLFW_KEY_ESCAPE:
            running = 0;
            break;
        case GLFW_KEY_PAGE_UP:
        case GLFW_KEY_Q:
            camera_get_up(&viewer->camera, axis);
            scale3v(axis, 0.1);
            camera_move(&viewer->camera, axis);
            break;
        case GLFW_KEY_PAGE_DOWN:
        case GLFW_KEY_E:
            camera_get_up(&viewer->camera, axis);
            scale3v(axis, -0.1);
            camera_move(&viewer->camera, axis);
            break;
        case GLFW_KEY_LEFT:
        case GLFW_KEY_A:
            camera_get_right(&viewer->camera, axis);
            scale3v(axis, -0.1);
            camera_move(&viewer->camera, axis);
            break;
        case GLFW_KEY_RIGHT:
        case GLFW_KEY_D:
            camera_get_right(&viewer->camera, axis);
            scale3v(axis, 0.1);
            camera_move(&viewer->camera, axis);
            break;
        case GLFW_KEY_DOWN:
        case GLFW_KEY_S:
            camera_get_backward(&viewer->camera, axis);
            scale3v(axis, 0.1);
            camera_move(&viewer->camera, axis);
            break;
        case GLFW_KEY_UP:
        case GLFW_KEY_W:
            camera_get_backward(&viewer->camera, axis);
            scale3v(axis, -0.1);
            camera_move(&viewer->camera, axis);
            break;
        case GLFW_KEY_F12:
            viewer_screenshot(viewer, "screenshot.png");
            break;
    }
}

static void close_callback(struct Viewer* viewer, void* userData) {
    running = 0;
}

int main() {
    struct Viewer *viewer, *viewer2;
    double t = 0.0, dt;
    struct Mesh cubeMesh = {0}, boxMesh = {0}, icosphereMesh = {0};
    struct GLObject cubeGl = {0}, boxGl = {0}, icosphereGl = {0}, cubeGl2 = {0};
    struct Geometry *sphere, *texturedCube, *coloredBox, *cube2;
    struct Scene scene, scene2;
    struct Node nodeCube;
    struct SolidColorMaterial sphereMat;
    struct PhongMaterial cubeMat = {
        {1.0, 1.0, 1.0},
        {1.0, 1.0, 1.0},
        {1.0, 1.0, 1.0},
        1.0
    };

    asset_manager_add_path(".");
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

    make_obj(&cubeMesh, "models/cube.obj", 0, 1, 1);
    make_box(&boxMesh, 4.0, 1.0, 1.0);
    make_icosphere(&icosphereMesh, 0.5, 2);
    viewer_make_current(viewer);
    globject_new(&cubeMesh, &cubeGl);
    globject_new(&boxMesh, &boxGl);
    globject_new(&icosphereMesh, &icosphereGl);
    viewer_make_current(viewer2);
    globject_new(&cubeMesh, &cubeGl2);
    mesh_free(&cubeMesh);
    mesh_free(&boxMesh);
    mesh_free(&icosphereMesh);

    viewer_make_current(viewer);
    sphere = solid_color_geometry_shared(&icosphereGl, &sphereMat);
    coloredBox = phong_color_geometry(&boxGl, 1.0, 1.0, 1.0, &cubeMat);
    texturedCube = phong_texture_geometry(&cubeGl, texture_load_from_file("test/assets/png/rgb_tux.png"), &cubeMat);
    viewer_make_current(viewer2);
    cube2 = solid_texture_geometry(&cubeGl, texture_load_from_file("test/assets/png/rgb_tux.png"));

    viewer_make_current(viewer);
    scene_init(&scene);
    spheres_and_boxes(sphere, texturedCube, &scene.root);

    test_init_local_light(&scene.lights.local[0]);
    scene.lights.numLocal = 1;

    viewer_make_current(viewer2);
    scene_init(&scene2);
    node_init(&nodeCube, cube2);
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
        hsv2rgb(fmod(t, 360.0), 1.0, 1.0, sphereMat.color);
        mul3sv(scene.lights.local[0].ambient, 0.1, sphereMat.color);
        mul3sv(scene.lights.local[0].diffuse, 1.0, sphereMat.color);
        scene_render(&scene, &viewer->camera);

        viewer_make_current(viewer2);
        viewer_next_frame(viewer2);
        scene_render(&scene2, &viewer2->camera);
    }

    free(sphere);
    free(texturedCube);
    free(coloredBox);
    free(cube2);
    globject_free(&cubeGl);
    globject_free(&boxGl);
    globject_free(&icosphereGl);
    viewer_free(viewer);
    scene_free(&scene);

    return 0;
}
