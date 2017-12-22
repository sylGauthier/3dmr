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
#include "shader.h"
#include "shaders/solid_color.h"
#include "shaders/solid_texture.h"

Viewer* viewer;
int running;

static void cursor_callback(double xpos, double ypos, double dx, double dy, int buttonLeft, int buttonMiddle, int buttonRight, void* data) {
    Mat3 rot, a, b;
    Vec3 x = {0, 1, 0}, y = {1, 0, 0};
    void* cubeModel = ((void**)data)[0];
    void* texturedCubeModel = ((void**)data)[1];

    if (buttonLeft) {
        mat4to3(a, cubeModel);
        load_rot3(rot, x, 4.0 * dx / viewer_get_width(viewer));
        mul3mm(b, rot, a);
        load_rot3(rot, y, 4.0 * dy / viewer_get_height(viewer));
        mul3mm(a, rot, b);
        mat3to4(cubeModel, a);
        mat4to3(a, texturedCubeModel);
        load_rot3(rot, x, -4.0 * dx / viewer_get_width(viewer));
        mul3mm(b, rot, a);
        load_rot3(rot, y, -4.0 * dy / viewer_get_height(viewer));
        mul3mm(a, rot, b);
        mat3to4(texturedCubeModel, a);
    }
}

static void wheel_callback(double xoffset, double yoffset, void* userData) {
    Vec3 t;
    struct Camera* camera = viewer_get_camera(viewer);
    
    t[0] = 0;
    t[1] = 0;
    t[2] = -yoffset;
    camera_move(camera, t);
    camera_update_view(camera);
}

static void key_callback(int key, int scancode, int action, int mods, void* userData) {
    if (action != GLFW_PRESS) {
        return;
    }
    switch (key) {
        case GLFW_KEY_ESCAPE:
            running = 0;
            break;
    }
}

static void close_callback(void* userData) {
    running = 0;
}

int main() {
    double current, dt, lastTime;
    struct Mesh cubeMesh = {0};
    struct SolidColorGeometry cube = {0};
    struct SolidTextureGeometry texturedCube = {0};
    struct Camera* camera;
    void* data[2];
    Vec3 t = {0, 0, 10};

    viewer = viewer_new(1024, 768, "Game");
    data[0] = cube.geometry.model;
    data[1] = texturedCube.geometry.model;
    viewer_set_callbacks(viewer, cursor_callback, wheel_callback, key_callback, close_callback, data);
    running = 1;
    lastTime = glfwGetTime();

    mesh_load(&cubeMesh, "models/cube.obj", 0, 0, 1);
    globject_new(&cubeMesh, &cube.geometry.glObject);
    load_id4(cube.geometry.model);
    cube.geometry.shader = shader_compile("shaders/solid_color.vert", "shaders/solid_color.frag");
    cube.geometry.render = draw_solid_color;
    cube.color[0] = 0.0;
    cube.color[1] = 0.0;
    cube.color[2] = 1.0;

    texturedCube.geometry.glObject = cube.geometry.glObject;
    load_id4(texturedCube.geometry.model);
    texturedCube.geometry.model[3][1] = 3.0;
    texturedCube.geometry.shader = shader_compile("shaders/solid_texture.vert", "shaders/solid_texture.frag");
    texturedCube.geometry.render = draw_solid_texture;
    texturedCube.texture = texture_load_from_file("textures/tux.png");

    camera = viewer_get_camera(viewer);
    camera_move(camera, t);
    camera_update_view(camera);

    while (running) {
        current = glfwGetTime();
        dt = current - lastTime;
        lastTime = current;
        viewer_process_events(viewer);
        usleep(10 * 1000);

        viewer_next_frame(viewer);
        geometry_render(&cube.geometry, camera);
        geometry_render(&texturedCube.geometry, camera);
    }

    glDeleteProgram(cube.geometry.shader);
    globject_free(&cube.geometry.glObject);
    mesh_free(&cubeMesh);
    viewer_free(viewer);

    return 0;
}
