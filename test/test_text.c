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
#include "test/util/main.h"

static struct GLObject textObj = {0};

int run(struct Viewer* viewer, struct Scene* scene, int argc, char** argv) {
    const char* text = "Hello world!";
    struct BitmapFont* font;
    struct Mesh textMesh;
    char* ttf;
    int err;

    if (argc) {
        text = argv[0];
    }

    viewer->cursor_callback = cursor_rotate_camera;
    viewer->wheel_callback = wheel_callback;
    viewer->callbackData = &scene->root;

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

    scene->root.object = &textObj;

    mesh_free(&textMesh);

    return EXIT_SUCCESS;
}
