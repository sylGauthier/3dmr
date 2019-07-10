#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <game/init.h>
#include <game/render/viewer.h>
#include <game/scene/scene.h>
#include "scenes.h"

static void usage(const char* prog) {
    printf("Usage: %s sceneName [screenshot]\n", prog);
}

void key_callback(struct Viewer* viewer, int key, int scancode, int action, int mods, void* d) {
    if (key == GLFW_KEY_ESCAPE) {
        *(int*)d = 0;
    }
}

static void close_callback(struct Viewer* viewer, void* d) {
    *(int*)d = 0;
}

static int cd_test(const char* prog) {
    const char* end;
    char* path;
    int r;
    end = strrchr(prog, '/');
    if (!end) return 1;
    if (!(path = malloc(end - prog + 1))) return 0;
    memcpy(path, prog, end - prog);
    path[end - prog] = 0;
    r = !chdir(path);
    free(path);
    return r;
}

struct Node* create_node(const struct Mesh* mesh, void* material) {
    struct Node* n;
    struct VertexArray* va;
    struct GLObject* glo;

    if (!material) return NULL;
    if (!(n = malloc(sizeof(*n) + sizeof(struct VertexArray) + sizeof(struct GLObject)))) return NULL;
    va = (void*)(n + 1);
    glo = (void*)(va + 1);
    vertex_array_gen(mesh, va);
    glo->vertexArray = va;
    glo->material = material;
    node_init(n);
    node_set_geometry(n, glo);
    return n;
}

void free_node(struct Node* node) {
    if (node->type == NODE_GEOMETRY) {
        vertex_array_del(node->data.geometry->vertexArray);
        free(node->data.geometry->material);
        free(node);
    }
}

int main(int argc, char** argv) {
    struct Scene scene;
    struct Viewer* viewer = NULL;
    unsigned int i;
    int ret = 1, sceneInit = 0;

    if (argc != 2 && argc != 3) {
        usage(argv[0]);
        return (argc > 1);
    }

    for (i = 0; i < NUM_DEMO_SCENES && strcmp(argv[1], scenes[i].name); i++);
    if (i >= NUM_DEMO_SCENES) {
        usage(argv[0]);
        return 1;
    }

    if (!cd_test(argv[0])) {
        fprintf(stderr, "Error: failed to cd to test dir\n");
        return 1;
    }

    if (!game_init("../shaders")) {
        fprintf(stderr, "Error: failed to init library\n");
        return 0;
    }

    if (!(viewer = viewer_new(640, 480, argv[1]))) {
        fprintf(stderr, "Error: failed to create viewer\n");
    } else if (!(sceneInit = scene_init(&scene, &viewer->camera))) {
        fprintf(stderr, "Error: failed to init scene\n");
    } else if (!scenes[i].setup(&scene)) {
        fprintf(stderr, "Error: failed to init scene %s\n", scenes[i].name);
    } else {
        double t = 0, dt;
        int running = 1;
        sceneInit++;
        glfwSwapInterval(1);
        viewer->key_callback = key_callback;
        viewer->close_callback = close_callback;
        viewer->callbackData = &running;
        while (running && (argc < 3 || t < 1)) {
            dt = viewer_next_frame(viewer);
            t += dt;
            if (scene_update_nodes(&scene, NULL)) {
                scene_update_render_queue(&scene);
            }
            scene_render(&scene);
            viewer_process_events(viewer);
        }
        if (argc == 3) {
            ret = !viewer_screenshot(viewer, argv[2]);
        } else {
            ret = 0;
        }
    }

    if (sceneInit > 1) scenes[i].teardown(&scene);
    if (sceneInit) scene_free(&scene, free_node);
    game_free();
    return ret;
}
