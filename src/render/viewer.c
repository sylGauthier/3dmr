#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <game/render/viewer.h>
#include <game/img/png.h>

struct ViewerImpl {
    struct Viewer user;
    GLFWwindow* window;
    int hasLast;
    double lastX, lastY;
    double lastTime;
    GLuint* programs;
    unsigned int numPrograms;
};

static struct Viewer* currentViewer = NULL;
static unsigned int numPrograms = 0;

static void cursor_callback(GLFWwindow* window, double xpos, double ypos) {
    struct ViewerImpl* viewer = glfwGetWindowUserPointer(window);
    int buttonLeft = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    int buttonMiddle = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS;
    int buttonRight = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
    if (viewer->hasLast && viewer->user.cursor_callback) {
        viewer->user.cursor_callback(&viewer->user, xpos, ypos, xpos - viewer->lastX, ypos - viewer->lastY, buttonLeft, buttonMiddle, buttonRight, viewer->user.callbackData);
    }
    viewer->lastX = xpos;
    viewer->lastY = ypos;
    viewer->hasLast = 1;
}

static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    struct ViewerImpl* viewer = glfwGetWindowUserPointer(window);
    if (viewer->user.wheel_callback) {
        viewer->user.wheel_callback(&viewer->user, xoffset, yoffset, viewer->user.callbackData);
    }
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    struct ViewerImpl* viewer = glfwGetWindowUserPointer(window);
    if (viewer->user.key_callback) {
        viewer->user.key_callback(&viewer->user, key, scancode, action, mods, viewer->user.callbackData);
    }
}

static void window_size_callback(GLFWwindow* window, int width, int height) {
    struct ViewerImpl* viewer = glfwGetWindowUserPointer(window);
    viewer->user.width = width;
    viewer->user.height = height;
    if (viewer->user.resize_callback) {
        viewer->user.resize_callback(&viewer->user, viewer->user.callbackData);
    }
}

static void window_close_callback(GLFWwindow* window) {
    struct ViewerImpl* viewer = glfwGetWindowUserPointer(window);
    if (viewer->user.close_callback) {
        viewer->user.close_callback(&viewer->user, viewer->user.callbackData);
    }
}

struct Viewer* viewer_new(unsigned int width, unsigned int height, const char* title) {
    struct ViewerImpl* viewer;
    GLenum error;

    if (!(viewer = malloc(sizeof(struct ViewerImpl)))) {
        fprintf(stderr, "Error: memory allocation failed\n");
    } else if (!glfwInit()) {
        fprintf(stderr, "Error: GLFW3 initialization failed\n");
    } else {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
        glfwWindowHint(GLFW_SAMPLES, 4);
        if (!(viewer->window = glfwCreateWindow(width, height, title, NULL, NULL))) {
            fprintf(stderr, "Error: window creation failed\n");
        } else {
            glfwMakeContextCurrent(viewer->window);
            glfwSetWindowUserPointer(viewer->window, viewer);
            glfwSetKeyCallback(viewer->window, key_callback);
            glfwSetCursorPosCallback(viewer->window, cursor_callback);
            glfwSetScrollCallback(viewer->window, scroll_callback);
            glfwSetWindowSizeCallback(viewer->window, window_size_callback);
            glfwSetWindowCloseCallback(viewer->window, window_close_callback);
            /*glfwSetInputMode(viewer->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);*/
            glewExperimental = 1;
            if ((error = glGetError()) != GL_NO_ERROR) {
                fprintf(stderr, "Error: GL context setup failed\n");
            } else if ((error = glewInit()) != GLEW_OK) {
                fprintf(stderr, "Error: GLEW initialization failed\n");
            } else {
                viewer->user.cursor_callback = NULL;
                viewer->user.wheel_callback = NULL;
                viewer->user.key_callback = NULL;
                viewer->user.resize_callback = NULL;
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
                viewer->programs = NULL;
                viewer->numPrograms = 0;
                viewer_make_current(&viewer->user);
                return &viewer->user;
            }
        }
    }

    viewer_free(&viewer->user);
    return NULL;
}

void viewer_free(struct Viewer* viewer) {
    unsigned int i;

    if (viewer) {
        struct Viewer* current = currentViewer;
        if (current == viewer) current = NULL;
        viewer_make_current(viewer);
        for (i = 0; i < ((struct ViewerImpl*)viewer)->numPrograms; i++) {
            if (((struct ViewerImpl*)viewer)->programs[i]) {
                glDeleteProgram(((struct ViewerImpl*)viewer)->programs[i]);
            }
        }
        free(((struct ViewerImpl*)viewer)->programs);
        if (((struct ViewerImpl*)viewer)->window) {
            glfwDestroyWindow(((struct ViewerImpl*)viewer)->window);
            glfwTerminate();
        }
        free(viewer);
        viewer_make_current(current);
    }
}

unsigned int viewer_register_program_id(void) {
    if (numPrograms == ((unsigned int)-1)) {
        return -1;
    }
    return numPrograms++;
}

GLuint viewer_get_program(struct Viewer* viewer, unsigned int id) {
    if (((struct ViewerImpl*)viewer)->numPrograms <= id) return 0;
    return ((struct ViewerImpl*)viewer)->programs[id];
}

int viewer_set_program(struct Viewer* viewer, unsigned int id, GLuint program) {
    if (id >= numPrograms) return 0;
    if (((struct ViewerImpl*)viewer)->numPrograms <= id) {
        GLuint* tmp;
        unsigned int i;
        if (!(tmp = realloc(((struct ViewerImpl*)viewer)->programs, numPrograms * sizeof(*tmp)))) {
            return 0;
        }
        ((struct ViewerImpl*)viewer)->programs = tmp;
        for (i = ((struct ViewerImpl*)viewer)->numPrograms; i < numPrograms; i++) {
            ((struct ViewerImpl*)viewer)->programs[i] = 0;
        }
        ((struct ViewerImpl*)viewer)->numPrograms = numPrograms;
    }
    ((struct ViewerImpl*)viewer)->programs[id] = program;
    return 1;
}

void viewer_set_title(struct Viewer* viewer, const char* title) {
    glfwSetWindowTitle(((struct ViewerImpl*)viewer)->window, title);
}

void viewer_make_current(struct Viewer* viewer) {
    glfwMakeContextCurrent(viewer ? (((struct ViewerImpl*)viewer)->window) : ((GLFWwindow*)NULL));
    currentViewer = viewer;
}

struct Viewer* viewer_get_current(void) {
    return currentViewer;
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

int viewer_screenshot(struct Viewer* viewer, const char* filename) {
    unsigned char* data;
    int ret;

    if (!(data = malloc(3 * viewer->width * viewer->height))) {
        fprintf(stderr, "Error: cannot allocate memory for screenshot\n");
        return 0;
    }
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glPixelStorei(GL_PACK_ROW_LENGTH, 0);
    glReadPixels(0, 0, viewer->width, viewer->height, GL_RGB, GL_UNSIGNED_BYTE, data);
    ret = png_write(filename, 1, viewer->width, viewer->height, 3, 1, data);
    free(data);
    return ret;
}
