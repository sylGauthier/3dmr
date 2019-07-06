#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <game/math/linear_algebra.h>
#include <game/render/camera.h>
#include <game/render/camera_buffer_object.h>
#include <game/render/viewer.h>
#include <game/scene/scene.h>
#include "args.h"

int running;

void cursor_rotate_camera(struct Viewer* viewer, double xpos, double ypos, double dx, double dy, int buttonLeft, int buttonMiddle, int buttonRight, void* data) {
    Vec3 axis = {0, 1, 0};
    struct Config* config = data;

    if (buttonLeft) {
        camera_rotate(&viewer->camera, axis, dx / viewer->width);
        camera_get_right(&viewer->camera, axis);
        camera_rotate(&viewer->camera, axis, dy / viewer->height);
        camera_buffer_object_update_view(MAT_CONST_CAST(&viewer->camera.view), config->scene.uboCamera);
        config->cameraChanged = 1;
    }
}

void wheel_callback(struct Viewer* viewer, double xoffset, double yoffset, void* userData) {
    Vec3 axis;
    struct Config* config = userData;

    camera_get_backward(&viewer->camera, axis);
    scale3v(axis, -yoffset / 10.0);
    camera_move(&viewer->camera, axis);
    camera_buffer_object_update_view(MAT_CONST_CAST(&viewer->camera.view), config->scene.uboCamera);
    camera_buffer_object_update_position(viewer->camera.position, config->scene.uboCamera);
    config->cameraChanged = 1;
}

void key_callback(struct Viewer* viewer, int key, int scancode, int action, int mods, void* userData) {
    Vec3 axis = {0, 1, 0};
    struct Config* config = userData;

    switch (key) {
        case GLFW_KEY_ESCAPE:
            running = 0;
            break;
        case GLFW_KEY_PAGE_UP:
        case GLFW_KEY_Q:
            camera_get_up(&viewer->camera, axis);
            scale3v(axis, 0.1);
            camera_move(&viewer->camera, axis);
            config->cameraChanged = 1;
            break;
        case GLFW_KEY_PAGE_DOWN:
        case GLFW_KEY_E:
            camera_get_up(&viewer->camera, axis);
            scale3v(axis, -0.1);
            camera_move(&viewer->camera, axis);
            config->cameraChanged = 1;
            break;
        case GLFW_KEY_LEFT:
        case GLFW_KEY_A:
            camera_get_right(&viewer->camera, axis);
            scale3v(axis, -0.1);
            camera_move(&viewer->camera, axis);
            config->cameraChanged = 1;
            break;
        case GLFW_KEY_RIGHT:
        case GLFW_KEY_D:
            camera_get_right(&viewer->camera, axis);
            scale3v(axis, 0.1);
            camera_move(&viewer->camera, axis);
            config->cameraChanged = 1;
            break;
        case GLFW_KEY_DOWN:
        case GLFW_KEY_S:
            camera_get_backward(&viewer->camera, axis);
            scale3v(axis, 0.1);
            camera_move(&viewer->camera, axis);
            config->cameraChanged = 1;
            break;
        case GLFW_KEY_UP:
        case GLFW_KEY_W:
            camera_get_backward(&viewer->camera, axis);
            scale3v(axis, -0.1);
            camera_move(&viewer->camera, axis);
            config->cameraChanged = 1;
            break;
        case GLFW_KEY_F12:
            viewer_screenshot(viewer, "screenshot.png");
            break;
    }

    if (config->cameraChanged) {
        camera_buffer_object_update_view(MAT_CONST_CAST(&viewer->camera.view), config->scene.uboCamera);
        camera_buffer_object_update_position(viewer->camera.position, config->scene.uboCamera);
    }
}

void resize_callback(struct Viewer* viewer, void* userData) {
    struct Config* config = userData;
    camera_buffer_object_update_projection(MAT_CONST_CAST(&viewer->camera.projection), config->scene.uboCamera);
    config->cameraChanged = 1;
}

void close_callback(struct Viewer* viewer, void* userData) {
    running = 0;
}

