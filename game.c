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
#include "geometry/solid_color.h"
#include "geometry/solid_texture.h"
#include "geometry/phong_color.h"
#include "geometry/phong_texture.h"
#include "mesh/obj.h"
#include "mesh/icosphere.h"

struct Viewer* viewer;
int running;

static void cursor_rotate_object(double xpos, double ypos, double dx, double dy, int buttonLeft, int buttonMiddle, int buttonRight, void* data) {
    Mat3 rot, a, b;
    Vec3 x = {0, 1, 0}, y = {1, 0, 0};

    if (buttonLeft) {
        mat4to3(a, data);
        load_rot3(rot, x, 4.0 * dx / viewer->width);
        mul3mm(b, rot, a);
        load_rot3(rot, y, 4.0 * dy / viewer->height);
        mul3mm(a, rot, b);
        mat3to4(data, a);
    }
}

static void cursor_rotate_camera(double xpos, double ypos, double dx, double dy, int buttonLeft, int buttonMiddle, int buttonRight, void* data) {
    Vec3 axis = {0, 1, 0};

    if (buttonLeft) {
        camera_rotate(&viewer->camera, axis, dx / viewer->width);
        camera_get_right(&viewer->camera, axis);
        camera_rotate(&viewer->camera, axis, dy / viewer->height);
    }
}

static void wheel_callback(double xoffset, double yoffset, void* userData) {
    Vec3 axis;
    
    camera_get_backward(&viewer->camera, axis);
    scale3v(axis, -yoffset);
    camera_move(&viewer->camera, axis);
}

static void key_callback(int key, int scancode, int action, int mods, void* userData) {
    Vec3 axis = {0,1,0};

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
    }
}

static void close_callback(void* userData) {
    running = 0;
}

void hsv2rgb(double h, double s, double v, Vec3 dest) {
    double p, q, t, rem;
    long i;

    if (s == 0.0) {
        dest[0] = v;
        dest[1] = v;
        dest[2] = v;
    } else {
        h /= 60.0;
        i = (long)h;
        rem = h - i;
        p = v * (1.0 - s);
        q = v * (1.0 - (s * rem));
        t = v * (1.0 - (s * (1.0 - rem)));

        switch (i) {
            case 0: dest[0] = v; dest[1] = t; dest[2] = p; break;
            case 1: dest[0] = q; dest[1] = v; dest[2] = p; break;
            case 2: dest[0] = p; dest[1] = v; dest[2] = t; break;
            case 3: dest[0] = p; dest[1] = q; dest[2] = v; break;
            case 4: dest[0] = t; dest[1] = p; dest[2] = v; break;
            case 5: dest[0] = v; dest[1] = p; dest[2] = q; break;
        }
    }
}

int main() {
    double t = 0.0, dt;
    struct Mesh cubeMesh = {0}, icosphereMesh = {0};
    struct GLObject cubeGl = {0}, icosphereGl = {0};
    struct Geometry *sphere, *texturedCube;
    struct Scene scene;
    struct Node cube, lamp;
    struct SolidColorMaterial sphereMat;
    struct PhongTextureMaterial cubeMat = {
        0,
        {
            {1.0, 1.0, 1.0},
            {1.0, 1.0, 1.0},
            {1.0, 1.0, 1.0},
            1.0
        }
    };

    viewer = viewer_new(1024, 768, "Game");
    viewer->cursor_callback = cursor_rotate_object;
    viewer->wheel_callback = wheel_callback;
    viewer->key_callback = key_callback;
    viewer->close_callback = close_callback;
    running = 1;

    obj_mesh(&cubeMesh, "models/cube.obj", 0, 1, 1);
    globject_new(&cubeMesh, &cubeGl);
    mesh_free(&cubeMesh);
    icosphere(&icosphereMesh, 1.0, 2);
    globject_new(&icosphereMesh, &icosphereGl);
    mesh_free(&icosphereMesh);

    /* cube = solid_color_geometry(&cubeGl, 0.0, 0.0, 1.0); */
    sphere = solid_color_geometry_shared(&icosphereGl, &sphereMat);
    texturedCube = phong_texture_geometry_shared(&cubeGl, &cubeMat);
    cubeMat.texture = texture_load_from_file("textures/tux.png");

    scene_init(&scene);
    node_init(&lamp);
    node_init(&cube);

    scene.lights.directional[0].direction[0] = 0;
    scene.lights.directional[0].direction[1] = 1;
    scene.lights.directional[0].direction[2] = 0;
    scene.lights.directional[0].ambient[0] = 0.1;
    scene.lights.directional[0].ambient[1] = 0.1;
    scene.lights.directional[0].ambient[2] = 0.1;
    scene.lights.directional[0].diffuse[0] = 0.5;
    scene.lights.directional[0].diffuse[1] = 0.5;
    scene.lights.directional[0].diffuse[2] = 0.5;
    scene.lights.directional[0].specular[0] = 0.2;
    scene.lights.directional[0].specular[1] = 0.2;
    scene.lights.directional[0].specular[2] = 0.2;
    scene.lights.numDirectional = 1;

    lamp.geometry = sphere;
    cube.geometry = texturedCube;
    cube.transform[3][1] = 3.0;

    scene_add(&scene, &lamp);
    scene_add(&scene, &cube);

    viewer->callbackData = cube.transform;

    while (running) {
        viewer_process_events(viewer);
        usleep(10 * 1000);

        dt = viewer_next_frame(viewer);
        t += 50.0 * dt;
        hsv2rgb(fmod(t, 360.0), 1.0, 1.0, sphereMat.color);
        mul3sv(scene.lights.directional[0].ambient, 0.1, sphereMat.color);
        mul3sv(scene.lights.directional[0].diffuse, 0.5, sphereMat.color);
        scene_render(&scene, &viewer->camera);
    }

    solid_color_shader_free();
    phong_texture_shader_free();
    free(sphere);
    free(texturedCube);
    globject_free(&cubeGl);
    globject_free(&icosphereGl);
    viewer_free(viewer);
    scene_free(&scene);

    return 0;
}
