#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <unistd.h>

#include <game/asset_manager.h>
#include <game/material/solid_text.h>
#include <game/render/camera.h>
#include <game/render/globject.h>
#include <game/render/viewer.h>
#include <game/scene/scene.h>
#include <game/text.h>

#include "test/util/callbacks.h"

int run(const char* text) {
    struct Viewer* viewer;
    struct Scene scene;
    struct BitmapFont* font;
    struct Mesh textMesh;
    struct GLObject textObj;
    struct Node textNode;
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

    if (!(ttf = asset_manager_find_file("font/FreeSans.ttf"))) {
        fprintf(stderr, "Failed to find font: %s\n", "font/FreeSans.ttf");
        return 1;
    }
    if (!(font = ttf_bitmap_font(ttf, 32))) {
        fprintf(stderr, "Failed to initialise font\n");
        return 1;
    }

    if ((err = new_text(font, text, &textMesh))) {
        fprintf(stderr, "Failed to create text mesh\n");
        return err;
    }
    textObj.vertexArray = vertex_array_new(&textMesh);

    if (!(textObj.material = (struct Material*)solid_text_material_new(1.0, 1.0, 1.0, font))) {
        fprintf(stderr, "Failed to create text material\n");
        return err;
    }

    node_init(&textNode, &textObj);
    scene_add(&scene, &textNode);

    while (running) {
        viewer_process_events(viewer);
        usleep(10 * 1000);
        viewer_next_frame(viewer);
        scene_render(&scene, &viewer->camera);
    }

    scene_free(&scene);
    free(textObj.material);
    vertex_array_free(textObj.vertexArray);
    mesh_free(&textMesh);
    font_free(font);
    viewer_free(viewer);

    return EXIT_SUCCESS;
}

void usage() {
    puts("Usage: test_text");
}

int main(int argc, char** argv) {
    const char* text = "Hello world!";
    if (argc > 1) {
        usage();
        return EXIT_FAILURE;
    }
    return run(text);
}
