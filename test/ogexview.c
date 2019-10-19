#include <stdlib.h>
#include <string.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <game/init.h>
#include <game/render/camera_buffer_object.h>
#include <game/render/lights_buffer_object.h>
#include <game/scene/opengex.h>

struct Prog {
    struct Scene scene;
    struct ImportMetadata metadata;
    unsigned int activeCam, numDirectionalLights, numPointLights;
    struct Node *dlights[MAX_DIRECTIONAL_LIGHTS], *plights[MAX_POINT_LIGHTS];
    int running;
};

static void usage(const char* prog) {
    printf("Usage: %s file\n", prog);
}

static void free_node_callback(struct Node* node) {
    if (node->type == NODE_GEOMETRY) {
        free(node->data.geometry);
    }
    if (node->father) {
        free(node);
    }
}

static void resize_callback(struct Viewer* viewer, void* data) {
    struct Prog* prog = data;
    struct Camera* activeCam = prog->metadata.cameraNodes[prog->activeCam]->data.camera;
    glViewport(0, 0, viewer->width, viewer->height);
    camera_set_ratio(((float)viewer->width) / ((float)viewer->height), activeCam->projection);
    camera_buffer_object_update_projection(&prog->scene.camera, MAT_CONST_CAST(activeCam->projection));
    uniform_buffer_send(&prog->scene.camera);
}

static void update_cam(struct Viewer* viewer, struct Prog* prog) {
    struct Camera* activeCam;
    activeCam = prog->metadata.cameraNodes[prog->activeCam]->data.camera;
    camera_set_ratio(((float)viewer->width) / ((float)viewer->height), activeCam->projection);
    camera_buffer_object_update_projection(&prog->scene.camera, MAT_CONST_CAST(activeCam->projection));
    camera_buffer_object_update_view_and_position(&prog->scene.camera, MAT_CONST_CAST(activeCam->view));
    uniform_buffer_send(&prog->scene.camera);
}

static void key_callback(struct Viewer* viewer, int key, int scancode, int action, int mods, void* data) {
    struct Prog* prog = data;
    if (action != GLFW_PRESS) return;
    switch (key) {
        case GLFW_KEY_ESCAPE:
            prog->running = 0;
            break;
        case GLFW_KEY_LEFT: case GLFW_KEY_UP:
            prog->activeCam = (prog->activeCam + prog->metadata.nbCameraNodes - 1) % prog->metadata.nbCameraNodes;
            update_cam(viewer, prog);
            break;
        case GLFW_KEY_RIGHT: case GLFW_KEY_DOWN:
            prog->activeCam = (prog->activeCam + 1) % prog->metadata.nbCameraNodes;
            update_cam(viewer, prog);
            break;
    }
}

static void close_callback(struct Viewer* viewer, void* data) {
    struct Prog* prog = data;
    prog->running = 0;
}

static void update_node(struct Scene* scene, struct Node* n, void* data) {
    struct Prog* prog = data;
    unsigned int i;

    switch (n->type) {
        case NODE_DLIGHT:
            for (i = 0; i < prog->numDirectionalLights; i++) {
                if (n == prog->dlights[i]) {
                    lights_buffer_object_update_dlight(&scene->lights, n->data.dlight, i);
                }
            }
            break;
        case NODE_PLIGHT:
            for (i = 0; i < prog->numPointLights; i++) {
                if (n == prog->plights[i]) {
                    lights_buffer_object_update_plight(&scene->lights, n->data.plight, i);
                }
            }
            break;
        case NODE_CAMERA:
            if (n == prog->metadata.cameraNodes[prog->activeCam]) {
                camera_buffer_object_update_view(&scene->camera, MAT_CONST_CAST(n->data.camera->view));
                camera_buffer_object_update_position(&scene->camera, n->position);
            }
            break;
        default:;
    }
}

static char* dirname(char* path) {
    char* s;
    if ((s = strrchr(path, '/'))) {
        *s = 0;
        return path;
    }
    return NULL;
}

int main(int argc, char** argv) {
    FILE* f = NULL;
    struct Prog prog;
    struct SharedData shared;
    struct Viewer* viewer = NULL;
    struct Camera* camera;
    unsigned int i;
    int sceneInit = 0, ogexInit = 0, err = 1;
    double dt;

    if (argc != 2) {
        usage(argv[0]);
        return 1;
    }
    if (!game_init(GAME_SHADERS_PATH)) {
        fprintf(stderr, "Error: failed to init game library\n");
        return 1;
    }

    if (!(viewer = viewer_new(1024, 768, argv[1]))) {
        fprintf(stderr, "Error: failed to start viewer\n");
    } else if (!(f = fopen(argv[1], "r"))) {
        fprintf(stderr, "Error: failed to open '%s'\n", argv[1]);
    } else if (!(sceneInit = scene_init(&prog.scene, NULL))) {
        fprintf(stderr, "Error: failed to init scene\n");
    } else if (!(ogexInit = ogex_load(&prog.scene.root, f, dirname(argv[1]), &shared, &prog.metadata))) {
        fprintf(stderr, "Error: failed to load scene '%s'\n", argv[1]);
    } else if (!prog.metadata.nbCameraNodes) {
        fprintf(stderr, "Error: no camera node in '%s'\n", argv[1]);
    } else {
        err = 0;
        prog.activeCam = 0;
        prog.running = 1;
        viewer->callbackData = &prog;
        viewer->resize_callback = resize_callback;
        viewer->key_callback = key_callback;
        viewer->close_callback = close_callback;
        scene_update_nodes(&prog.scene, NULL, NULL);
        for (i = 0; i < prog.metadata.nbLightNodes; i++) {
            struct Node* n = prog.metadata.lightNodes[i];
            switch (n->type) {
                case NODE_DLIGHT:
                    if (prog.numDirectionalLights < MAX_DIRECTIONAL_LIGHTS) {
                        lights_buffer_object_update_dlight(&prog.scene.lights, n->data.dlight, prog.numDirectionalLights);
                        prog.dlights[prog.numDirectionalLights++] = n;
                    } else {
                        fprintf(stderr, "Warning: directional lights limit exceeded\n");
                    }
                    break;
                case NODE_PLIGHT:
                    if (prog.numPointLights < MAX_POINT_LIGHTS) {
                        lights_buffer_object_update_plight(&prog.scene.lights, n->data.plight, prog.numPointLights);
                        prog.plights[prog.numPointLights++] = n;
                    } else {
                        fprintf(stderr, "Warning: point lights limit exceeded\n");
                    }
                    break;
                default:;
            }
        }
        for (i = 0; i < prog.metadata.nbClips; i++) {
            prog.metadata.clips[i].loop = 1;
            prog.metadata.clips[i].mode = CLIP_FORWARD;
        }
        lights_buffer_object_update_ndlight(&prog.scene.lights, prog.numDirectionalLights);
        lights_buffer_object_update_nplight(&prog.scene.lights, prog.numPointLights);
        camera = prog.metadata.cameraNodes[0]->data.camera;
        camera_set_ratio(((float)viewer->width) / ((float)viewer->height), camera->projection);
        camera_buffer_object_update_projection(&prog.scene.camera, MAT_CONST_CAST(camera->projection));
        camera_buffer_object_update_view_and_position(&prog.scene.camera, MAT_CONST_CAST(camera->view));
        glfwSwapInterval(1);
        while (prog.running) {
            viewer_process_events(viewer);
            dt = viewer_next_frame(viewer);
            if (prog.metadata.nbClips) {
                for (i = 0; i < prog.metadata.nbClips; i++) {
                    anim_play_clip(prog.metadata.clips + i, dt * 1000);
                }
                scene_update_nodes(&prog.scene, update_node, &prog);
                uniform_buffer_send(&prog.scene.lights);
                uniform_buffer_send(&prog.scene.camera);
            }
            camera = prog.metadata.cameraNodes[prog.activeCam]->data.camera;
            scene_update_render_queue(&prog.scene, MAT_CONST_CAST(camera->view), MAT_CONST_CAST(camera->projection));
            scene_render(&prog.scene);
        }
    }

    if (f) fclose(f);
    if (sceneInit) scene_free(&prog.scene, free_node_callback);
    if (ogexInit) {
        import_free_shared_data(&shared);
        import_free_metadata(&prog.metadata);
    }
    if (viewer) viewer_free(viewer);
    game_free();

    return err;
}
