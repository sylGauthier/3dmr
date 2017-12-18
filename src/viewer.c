#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "viewer.h"
#include "camera.h"

struct Viewer {
    GLFWwindow* window;
    void (*cursor_callback)(double, double, double, double, int, int, int, void*);
    void (*wheel_callback)(double, double, void*);
    void (*key_callback)(int, int, int, int, void*);
    void (*close_callback)(void*);
    void* userData;
    int hasLast;
    double lastX, lastY;
    int width, height;
    struct Camera camera;

    double lastTime;
};

static void cursor_callback(GLFWwindow* window, double xpos, double ypos) {
    Viewer* viewer = glfwGetWindowUserPointer(window);
    int buttonLeft = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    int buttonMiddle = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS;
    int buttonRight = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
    if (viewer->hasLast && viewer->cursor_callback) {
        viewer->cursor_callback(xpos, ypos, xpos - viewer->lastX, ypos - viewer->lastY, buttonLeft, buttonMiddle, buttonRight, viewer->userData);
    }
    viewer->lastX = xpos;
    viewer->lastY = ypos;
    viewer->hasLast = 1;
}

static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    Viewer* viewer = glfwGetWindowUserPointer(window);
    if (viewer->wheel_callback) {
        viewer->wheel_callback(xoffset, yoffset, viewer->userData);
    }
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    Viewer* viewer = glfwGetWindowUserPointer(window);
    if (viewer->key_callback) {
        viewer->key_callback(key, scancode, action, mods, viewer->userData);
    }
}

static void window_size_callback(GLFWwindow* window, int width, int height) {
    Viewer* viewer = glfwGetWindowUserPointer(window);
    glViewport(0, 0, width, height);
    viewer->camera.ratio = ((float)width) / ((float)height);
    camera_update_projection(&viewer->camera);
    viewer->width = width;
    viewer->height = height;
}

static void window_close_callback(GLFWwindow* window) {
    Viewer* viewer = glfwGetWindowUserPointer(window);
    if (viewer->close_callback) {
        viewer->close_callback(viewer->userData);
    }
}

struct Viewer* viewer_new(unsigned int width, unsigned int height, const char* title) {
    struct Viewer* viewer;
    GLenum error;

    if ((viewer = malloc(sizeof(struct Viewer)))) {
        if (glfwInit()) {
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
            glfwWindowHint(GLFW_SAMPLES, 4);
            if ((viewer->window = glfwCreateWindow(width, height, title, NULL, NULL))) {
                glfwMakeContextCurrent(viewer->window);
                glfwSetWindowUserPointer(viewer->window, viewer);
                glfwSetKeyCallback(viewer->window, key_callback);
                glfwSetCursorPosCallback(viewer->window, cursor_callback);
                glfwSetScrollCallback(viewer->window, scroll_callback);
                glfwSetWindowSizeCallback(viewer->window, window_size_callback);
                glfwSetWindowCloseCallback(viewer->window, window_close_callback);
                /*glfwSetInputMode(viewer->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);*/
                if ((error = glGetError()) == GL_NO_ERROR) {
                    glewExperimental = 1;
                    if ((error = glewInit()) == GLEW_OK) {
                        Vec3 pos = {0, 0, 0};
                        camera_load_default(&viewer->camera, pos, ((float)width) / ((float)height));

                        viewer->cursor_callback = NULL;
                        viewer->wheel_callback = NULL;
                        viewer->key_callback = NULL;
                        viewer->close_callback = NULL;
                        viewer->userData = NULL;
                        viewer->hasLast = 0;
                        viewer->width = width;
                        viewer->height = height;
                        viewer->lastTime = glfwGetTime();

                        glEnable(GL_DEPTH_TEST);
                        glDepthFunc(GL_LESS);
                        glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
                        glEnable(GL_TEXTURE_2D);
                        glEnable(GL_MULTISAMPLE);
                        glEnable(GL_BLEND);
                        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                        return viewer;
                    } else {
                        fprintf(stderr, "GLEW initialization failed\n");
                    }
                } else {
                    fprintf(stderr, "GL context setup failed\n");
                }
            } else {
                fprintf(stderr, "Window creation failed\n");
            }
        } else {
            fprintf(stderr, "GLFW3 initialization failed\n");
        }
    } else {
        fprintf(stderr, "Memory error\n");
    }

    viewer_free(viewer);
	return NULL;
}

void viewer_free(struct Viewer* viewer) {
    if (viewer) {
        if (viewer->window) {
            glfwDestroyWindow(viewer->window);
            glfwTerminate();
        }
        free(viewer);
    }
}

void viewer_set_callbacks(struct Viewer* viewer, void (*cursor_callback)(double, double, double, double, int, int, int, void*), void (*wheel_callback)(double, double, void*), void (*key_callback)(int, int, int, int, void*), void (*close_callback)(void*), void* userData) {
    viewer->cursor_callback = cursor_callback;
    viewer->wheel_callback = wheel_callback;
    viewer->key_callback = key_callback;
    viewer->close_callback = close_callback;
    viewer->userData = userData;
}

void viewer_process_events(struct Viewer* viewer) {
    glfwPollEvents();
}

void viewer_next_frame(struct Viewer* viewer) {
    glfwSwapBuffers(viewer->window);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

struct Camera* viewer_get_camera(struct Viewer* viewer) {
    return &viewer->camera;
}

int viewer_get_width(struct Viewer* viewer) {
    return viewer->width;
}

int viewer_get_height(struct Viewer* viewer) {
    return viewer->height;
}
