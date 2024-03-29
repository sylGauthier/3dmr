#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <3dmr/shaders.h>
#include <3dmr/render/camera_buffer_object.h>
#include <3dmr/render/viewer.h>
#include <3dmr/scene/scene.h>
#include "scenes.h"

struct CallbackParam {
    int running;
    struct Camera* camera;
    struct UniformBuffer* cambuffer;
};

static void usage(const char* prog) {
    printf("Usage: %s [-hl] sceneName [screenshot]\n", prog);
}

static void list_scenes(void) {
    unsigned int i;

    for (i = 0; i < NUM_DEMO_SCENES; i++) {
        printf("%s\n", scenes[i].name);
    }
}

void key_callback(struct Viewer* viewer, int key, int scancode, int action, int mods, void* d) {
    if (key == GLFW_KEY_ESCAPE) {
        ((struct CallbackParam*)d)->running = 0;
    }
}

static void close_callback(struct Viewer* viewer, void* d) {
    ((struct CallbackParam*)d)->running = 0;
}

static void resize_callback(struct Viewer* viewer, void* d) {
    glViewport(0, 0, viewer->width, viewer->height);
    camera_projection(((float)viewer->width) / ((float)viewer->height), 1.04, 0.1, 2000, ((struct CallbackParam*)d)->camera->projection);
    camera_buffer_object_update_projection(((struct CallbackParam*)d)->cambuffer, MAT_CONST_CAST(((struct CallbackParam*)d)->camera->projection));
    uniform_buffer_send(((struct CallbackParam*)d)->cambuffer);
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

struct Node* create_node(const struct Mesh* mesh, struct Material* material) {
    struct Node* n;
    struct VertexArray* va;
    struct Geometry* geom;

    if (!material) return NULL;
    if (!(n = malloc(sizeof(*n) + sizeof(*va) + sizeof(*geom)))) return NULL;
    va = (void*)(n + 1);
    geom = (void*)(va + 1);
    vertex_array_gen(mesh, va);
    geom->vertexArray = va;
    geom->material = material;
    node_init(n);
    node_set_geometry(n, geom);
    return n;
}

void free_node(struct Node* node) {
    if (node->type == NODE_GEOMETRY) {
        vertex_array_del(node->data.geometry->vertexArray);
        free(node->data.geometry->material->params);
        free(node->data.geometry->material);
        free(node);
    }
}

int main(int argc, char** argv) {
    struct Scene scene;
    struct Camera camera;
    Quaternion corient = {1, 0, 0, 0};
    Vec3 cpos = {0, 0, 10};
    struct Viewer* viewer = NULL;
    unsigned int i;
    int ret = 1, sceneInit = 0;
    char *arg;

    for (i = 1; i < argc; i++) {
        arg = argv[i];
        if (arg[0] == '-' && arg[1] == '-' && arg[2] == '\0') {
            break; /* end of options */
        }
        if (arg[0] == '-' && arg[1] == '-' && arg[2] != '\0') {
            usage(argv[0]); /* long opt */
        }
        if (arg[0] == '-' && arg[1] != '-') {
            for (arg++; *arg; arg++) {
                switch (*arg) {
                    default:
                    case 'h':
                        usage(argv[0]);
                        return 0;
                    case 'l':
                        list_scenes();
                        return 0;
                }
            }
        }
    }

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
    tdmrShaderRootPath = "../shaders";

    camera_projection(640.0 / 480.0, 1.04, 0.1, 2000, camera.projection);
    camera_view(cpos, corient, camera.view);
    if (!(viewer = viewer_new(640, 480, argv[1]))) {
        fprintf(stderr, "Error: failed to create viewer\n");
    } else if (!(sceneInit = scene_init(&scene, &camera))) {
        fprintf(stderr, "Error: failed to init scene\n");
    } else if (!scenes[i].setup(&scene)) {
        fprintf(stderr, "Error: failed to init scene %s\n", scenes[i].name);
    } else {
        struct CallbackParam p;
        double t = 0, dt;
        p.running = 1;
        p.camera = &camera;
        p.cambuffer = &scene.camera;
        sceneInit++;
        glfwSwapInterval(1);
        viewer->key_callback = key_callback;
        viewer->close_callback = close_callback;
        viewer->resize_callback = resize_callback;
        viewer->callbackData = &p;
        while (p.running && (argc < 3 || t < 1)) {
            dt = viewer_next_frame(viewer);
            t += dt;
            if (scene_update_nodes(&scene, NULL, NULL)) {
                scene_update_render_queue(&scene, MAT_CONST_CAST(camera.view), MAT_CONST_CAST(camera.projection));
            }
            scene_render(&scene, NULL);
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
    return ret;
}
