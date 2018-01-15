#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "viewer.h"

struct ViewerImpl {
    struct Viewer user;
    GLFWwindow* window;
    int hasLast;
    double lastX, lastY;
    double lastTime;
};

static void cursor_callback(GLFWwindow* window, double xpos, double ypos) {
    struct ViewerImpl* viewer = glfwGetWindowUserPointer(window);
    int buttonLeft = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    int buttonMiddle = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS;
    int buttonRight = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
    if (viewer->hasLast && viewer->user.cursor_callback) {
        viewer->user.cursor_callback(xpos, ypos, xpos - viewer->lastX, ypos - viewer->lastY, buttonLeft, buttonMiddle, buttonRight, viewer->user.callbackData);
    }
    viewer->lastX = xpos;
    viewer->lastY = ypos;
    viewer->hasLast = 1;
}

static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    struct ViewerImpl* viewer = glfwGetWindowUserPointer(window);
    if (viewer->user.wheel_callback) {
        viewer->user.wheel_callback(xoffset, yoffset, viewer->user.callbackData);
    }
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    struct ViewerImpl* viewer = glfwGetWindowUserPointer(window);
    if (viewer->user.key_callback) {
        viewer->user.key_callback(key, scancode, action, mods, viewer->user.callbackData);
    }
}

static void window_size_callback(GLFWwindow* window, int width, int height) {
    struct ViewerImpl* viewer = glfwGetWindowUserPointer(window);
    glViewport(0, 0, width, height);
    viewer->user.camera.ratio = ((float)width) / ((float)height);
    camera_update_projection(&viewer->user.camera);
    viewer->user.width = width;
    viewer->user.height = height;
}

static void window_close_callback(GLFWwindow* window) {
    struct ViewerImpl* viewer = glfwGetWindowUserPointer(window);
    if (viewer->user.close_callback) {
        viewer->user.close_callback(viewer->user.callbackData);
    }
}

struct Viewer* viewer_new(unsigned int width, unsigned int height, const char* title) {
    struct ViewerImpl* viewer;
    GLenum error;

    if ((viewer = malloc(sizeof(struct ViewerImpl)))) {
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
                        Vec3 pos = {0, 0, 10};
                        camera_load_default(&viewer->user.camera, pos, ((float)width) / ((float)height));

                        viewer->user.cursor_callback = NULL;
                        viewer->user.wheel_callback = NULL;
                        viewer->user.key_callback = NULL;
                        viewer->user.close_callback = NULL;
                        viewer->user.callbackData = NULL;
                        viewer->user.width = width;
                        viewer->user.height = height;
                        viewer->hasLast = 0;
                        viewer->lastTime = glfwGetTime();

                        glEnable(GL_DEPTH_TEST);
                        glDepthFunc(GL_LESS);
                        glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
                        glEnable(GL_TEXTURE_2D);
                        glEnable(GL_MULTISAMPLE);
                        glEnable(GL_BLEND);
                        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                        return &viewer->user;
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

    viewer_free(&viewer->user);
    return NULL;
}

void viewer_free(struct Viewer* viewer) {
    if (viewer) {
        if (((struct ViewerImpl*)viewer)->window) {
            glfwDestroyWindow(((struct ViewerImpl*)viewer)->window);
            glfwTerminate();
        }
        free(viewer);
    }
}

void viewer_process_events(struct Viewer* viewer) {
    glfwPollEvents();
}

double viewer_next_frame(struct Viewer* viewer) {
    double current = glfwGetTime();
    double dt = current - ((struct ViewerImpl*)viewer)->lastTime;
    ((struct ViewerImpl*)viewer)->lastTime = current;
    glfwSwapBuffers(((struct ViewerImpl*)viewer)->window);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    return dt;
}
