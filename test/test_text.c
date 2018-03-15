#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <unistd.h>

#include "viewer.h"
#include "camera.h"
#include "scene.h"
#include "globject.h"
#include "text.h"
#include "shader.h"
#include "asset_manager.h"

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

static void text_prerender(const struct Geometry* geometry, const struct Camera* camera, const struct Lights* lights) {
    static const float color[3] = {1,1,1};
    glUniform3fv(glGetUniformLocation(geometry->shader, "textColor"), 1, color);
    glBindTexture(GL_TEXTURE_2D, ((struct BitmapFont *)geometry->material)->texture_atlas);
}

static void text_postrender(const struct Geometry* geometry, const struct Camera* camera, const struct Lights* lights) {
    glBindTexture(GL_TEXTURE_2D, 0);
}

int run(const char* text) {
    struct Viewer *viewer;
    struct Scene scene;
    struct BitmapFont* font;
    struct Mesh text_mesh;
    struct GLObject text_gl;
    struct Geometry text_geom;
    struct Node text_node;
    GLuint text_shader;
    char* ttf;
    int err;

    asset_manager_add_path("..");
    asset_manager_add_path("assets");
    asset_manager_add_path(".");
    asset_manager_add_path("test/assets");

    viewer = viewer_new(1024, 768, "test_text");
    viewer->cursor_callback = cursor_rotate_camera;
    viewer->wheel_callback = wheel_callback;
    viewer->key_callback = key_callback;
    viewer->close_callback = close_callback;
    running = 1;

    scene_init(&scene);
    viewer->callbackData = &scene.root;

    ttf = asset_manager_find_file("font/FreeSans.ttf");
    if (!ttf) {
	fprintf(stderr, "Failed to find font: %s\n", "font/FreeSans.ttf");
	return 1;
    }
    font = ttf_bitmap_font(ttf, 32);
    if (!font) {
	fprintf(stderr, "Failed to initialise font\n");
	return 1;
    }

    err = new_text(font, text, &text_mesh);
    if (err) {
	fprintf(stderr, "Failed to create text mesh\n");
	return err;
    }
    globject_new(&text_mesh, &text_gl);
    /* create geometry by hand */
    text_shader = asset_manager_load_shader("shaders/text.vert", "shaders/text.frag");

    text_geom.glObject = text_gl;
    text_geom.shader = text_shader;
    text_geom.mode = GL_FILL;
    text_geom.material = (void*) font;
    text_geom.prerender = text_prerender;
    text_geom.postrender = text_postrender;

    node_init(&text_node, &text_geom);
    scene_add(&scene, &text_node);

    while (running) {
        viewer_process_events(viewer);
        usleep(10 * 1000);
        viewer_next_frame(viewer);
        scene_render(&scene, &viewer->camera);
    }

    scene_free(&scene);
    globject_free(&text_gl);
    mesh_free(&text_mesh);
    font_free(font);
    viewer_free(viewer);

    return EXIT_SUCCESS;
}

void usage() {
    puts("Usage: test_text");
}

int main(int argc, char* argv[]) {
    char* text = "Hello world!";
    if (argc > 1) {
	usage();
	return EXIT_FAILURE;
    }
    return run(text);
}
