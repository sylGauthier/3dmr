#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "viewer.h"
#include "node.h"
#include "camera.h"
#include "linear_algebra.h"

int running;

void cursor_rotate_object(struct Viewer* viewer, double xpos, double ypos, double dx, double dy, int buttonLeft, int buttonMiddle, int buttonRight, void* data) {
    Vec3 x = {0, 1, 0}, y = {1, 0, 0};

    if (buttonLeft) {
        node_rotate(data, x, 4.0 * dx / viewer->width);
        node_rotate(data, y, 4.0 * dy / viewer->height);
    }
}

void cursor_rotate_camera(struct Viewer* viewer, double xpos, double ypos, double dx, double dy, int buttonLeft, int buttonMiddle, int buttonRight, void* data) {
    Vec3 axis = {0, 1, 0};

    if (buttonLeft) {
        camera_rotate(&viewer->camera, axis, dx / viewer->width);
        camera_get_right(&viewer->camera, axis);
        camera_rotate(&viewer->camera, axis, dy / viewer->height);
    }
}

void wheel_callback(struct Viewer* viewer, double xoffset, double yoffset, void* userData) {
    Vec3 axis;

    camera_get_backward(&viewer->camera, axis);
    scale3v(axis, -yoffset);
    camera_move(&viewer->camera, axis);
}

void key_callback(struct Viewer* viewer, int key, int scancode, int action, int mods, void* userData) {
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

void close_callback(struct Viewer* viewer, void* userData) {
    running = 0;
}

