#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <unistd.h>

#include "viewer.h"
#include "camera.h"
#include "mesh.h"
#include "globject.h"
#include "texture.h"
#include "geometries/solid_color.h"
#include "geometries/solid_texture.h"

struct Viewer* viewer;
int running;

static void cursor_callback(double xpos, double ypos, double dx, double dy, int buttonLeft, int buttonMiddle, int buttonRight, void* data) {
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

static void wheel_callback(double xoffset, double yoffset, void* userData) {
    Vec3 axis;
    
    camera_get_backward(&viewer->camera, axis);
    scale3v(axis, -yoffset);
    camera_move(&viewer->camera, axis);
}

static void key_callback(int key, int scancode, int action, int mods, void* userData) {
    Vec3 axis;

    switch (key) {
        case GLFW_KEY_ESCAPE:
            running = 0;
            break;
        case GLFW_KEY_LEFT:
        case GLFW_KEY_A:
            camera_get_up(&viewer->camera, axis);
            camera_rotate(&viewer->camera, axis, 0.01);
            break;
        case GLFW_KEY_RIGHT:
        case GLFW_KEY_D:
            camera_get_up(&viewer->camera, axis);
            camera_rotate(&viewer->camera, axis, -0.01);
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

#define A 0.6180339887498948f
#define B 1.0f
#define C 0.0f

static float vertices[] = {
    -A, B, C, A, B, C, -A, -B, C, A, -B, C,
    C, -A, B, C, A, B, C, -A, -B, C, A, -B,
    B, C, -A, B, C, A, -B, C, -A, -B, C, A
};

static unsigned int indices[] = {
    0, 11, 5, 0, 5, 1, 0, 1, 7, 0, 7, 10, 0, 10, 11,
    1, 5, 9, 5, 11, 4, 11, 10, 2, 10, 7, 6, 7, 1, 8,
    3, 9, 4, 3, 4, 2, 3, 2, 6, 3, 6, 8, 3, 8, 9,
    4, 9, 5, 2, 4, 11, 6, 2, 10, 8, 6, 7, 9, 8, 1
};

int main() {
    double dt;
    struct Mesh icosahedronMesh = {
        vertices,
        NULL,
        NULL,
        indices,
        12,
        0,
        0,
        60
    };
    struct GLObject icosahedronGl = {0};
    struct SolidColorGeometry icosahedron = {0};

    viewer = viewer_new(1024, 768, "Game");
    viewer->cursor_callback = cursor_callback;
    viewer->wheel_callback = wheel_callback;
    viewer->key_callback = key_callback;
    viewer->close_callback = close_callback;
    viewer->callbackData = icosahedron.geometry.model;
    running = 1;

    globject_new(&icosahedronMesh, &icosahedronGl);
    solid_color_geometry(&icosahedron, &icosahedronGl, 0.0, 0.0, 1.0);
    icosahedron.geometry.mode = GL_LINE;

    while (running) {
        viewer_process_events(viewer);
        usleep(10 * 1000);

        dt = viewer_next_frame(viewer);
        geometry_render(&icosahedron.geometry, &viewer->camera);
    }

    solid_color_shader_free();
    globject_free(&icosahedronGl);
    viewer_free(viewer);

    return 0;
}
