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
#include "shader.h"
#include "shaders/solid_color.h"

Viewer* viewer;
int running;
Mat4 cubeModel = {
    {1.0, 0.0, 0.0, 0.0},
    {0.0, 1.0, 0.0, 0.0},
    {0.0, 0.0, 1.0, 0.0},
    {0.0, 0.0, 0.0, 1.0}
};

static void cursor_callback(double xpos, double ypos, double dx, double dy, int buttonLeft, int buttonMiddle, int buttonRight, void* userData)
{
    Mat3 rot, a, b;
    Vec3 x = {0, 1, 0}, y = {1, 0, 0};

    if (buttonLeft)
    {
        mat4to3(a, cubeModel);
        load_rot3(rot, x, 4.0 * dx / viewer_get_width(viewer));
        mul3mm(b, rot, a);
        load_rot3(rot, y, 4.0 * dy / viewer_get_height(viewer));
        mul3mm(a, rot, b);
        mat3to4(cubeModel, a);
    }
}

static void wheel_callback(double xoffset, double yoffset, void* userData)
{
    Vec3 t;
    struct Camera* camera = viewer_get_camera(viewer);
    
    t[0] = 0;
    t[1] = 0;
    t[2] = -yoffset;
    camera_move(camera, t);
    camera_update_view(camera);
}

static void key_callback(int key, int scancode, int action, int mods, void* userData)
{
    if (action != GLFW_PRESS)
        return;
    switch (key)
    {
        case GLFW_KEY_ESCAPE:
            running = 0;
            break;
    }
}

static void close_callback(void* userData)
{
    running = 0;
}

int main()
{
    double current, dt, lastTime;
    GLuint shaderSolidColor;
    struct Mesh cubeMesh;
    struct GLObject cube;
    struct Camera* camera;
    float cubeColor[] = {0.0, 0.0, 1.0, 0.0};

    viewer = viewer_new(1024, 768, "Game");
    viewer_set_callbacks(viewer, cursor_callback, wheel_callback, key_callback, close_callback, NULL);
    running = 1;
    lastTime = glfwGetTime();

    shaderSolidColor = shader_compile("shaders/solid_color.vert", "shaders/solid_color.frag");
    mesh_load(&cubeMesh, "models", "cube.obj");
    globject_new(&cubeMesh, &cube);
    camera = viewer_get_camera(viewer);

    while (running) {
        current = glfwGetTime();
        dt = current - lastTime;
        lastTime = current;
        viewer_process_events(viewer);
        usleep(10 * 1000);

        viewer_next_frame(viewer);
        draw_solid_color(&cube, shaderSolidColor, camera, cubeModel, cubeColor);
    }

    globject_free(&cube);
    mesh_free(&cubeMesh);
    glDeleteProgram(shaderSolidColor);
    viewer_free(viewer);

    return 0;
}
