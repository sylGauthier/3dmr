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
    unsigned int activeCam, activeClip, numDirectionalLights, numPointLights, numSpotLights;
    struct Node *dlights[MAX_DIRECTIONAL_LIGHTS], *plights[MAX_POINT_LIGHTS], *slights[MAX_SPOT_LIGHTS];
    int running;
};

static void usage(const char* prog) {
    printf("Usage: %s file [file2 [file3 ...]]\n", prog);
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
    if (prog->metadata.cameraNodes[prog->activeCam]->name) {
        printf("Current camera: %s\n", prog->metadata.cameraNodes[prog->activeCam]->name);
    } else {
        printf("Current camera: #%u\n", prog->activeCam);
    }
}

static void key_callback(struct Viewer* viewer, int key, int scancode, int action, int mods, void* data) {
    struct Prog* prog = data;
    if (action != GLFW_PRESS) return;
    switch (key) {
        case GLFW_KEY_ESCAPE:
            prog->running = 0;
            break;
        case GLFW_KEY_LEFT: case GLFW_KEY_UP:
            prog->activeCam = (prog->activeCam + prog->metadata.numCameraNodes - 1) % prog->metadata.numCameraNodes;
            update_cam(viewer, prog);
            break;
        case GLFW_KEY_RIGHT: case GLFW_KEY_DOWN:
            prog->activeCam = (prog->activeCam + 1) % prog->metadata.numCameraNodes;
            update_cam(viewer, prog);
            break;
        case GLFW_KEY_SPACE:
            prog->activeClip = (prog->activeClip + 1) % prog->metadata.numClips;
            if (prog->metadata.clips[prog->activeClip]->name) {
                printf("Current clip: %s\n", prog->metadata.clips[prog->activeClip]->name);
            } else {
                printf("Current clip: #%u\n", prog->activeClip);
            }
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
        case NODE_SLIGHT:
            for (i = 0; i < prog->numSpotLights; i++) {
                if (n == prog->slights[i]) {
                    lights_buffer_object_update_slight(&scene->lights, n->data.slight, i);
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

static int load_scenes(struct Prog* prog, struct ImportSharedData* shared, int argc, char** argv) {
    FILE* file;
    unsigned int i;

    for (i = 0; i < argc; i++) {
        printf("Appending scene: %s\n", argv[i]);
        if (!(file = fopen(argv[i], "r"))) {
            fprintf(stderr, "Error: could not open file: %s\n", argv[i]);
            return 0;
        }
        if (!ogex_load(&prog->scene.root, file, dirname(argv[i]), shared, &prog->metadata)) {
            fprintf(stderr, "Error: failed to load ogex file\n");
            fclose(file);
            return 0;
        }
        fclose(file);
    }
    return 1;
}

int main(int argc, char** argv) {
    FILE* f = NULL;
    struct Prog prog;
    struct ImportSharedData shared;
    struct Viewer* viewer = NULL;
    struct Camera* camera;
    unsigned int i;
    int sceneInit = 0, ogexInit = 0, err = 1;
    double dt;

    if (argc < 2) {
        usage(argv[0]);
        return 1;
    }
    if (!game_init(GAME_SHADERS_PATH)) {
        fprintf(stderr, "Error: failed to init game library\n");
        return 1;
    }

    prog.numDirectionalLights = 0;
    prog.numPointLights = 0;
    prog.numSpotLights = 0;
    import_init_metadata(&prog.metadata);
    import_init_shared_data(&shared);
    if (!(viewer = viewer_new(1024, 768, argv[1]))) {
        fprintf(stderr, "Error: failed to start viewer\n");
    } else if (!(sceneInit = scene_init(&prog.scene, NULL))) {
        fprintf(stderr, "Error: failed to init scene\n");
    } else if (!(ogexInit = load_scenes(&prog, &shared, argc - 1, argv + 1))) {
        fprintf(stderr, "Error: failed to load scenes\n");
    } else if (!prog.metadata.numCameraNodes) {
        fprintf(stderr, "Error: no camera node in any of the scenes\n");
    } else {
        err = 0;
        prog.activeCam = 0;
        prog.activeClip = 0;
        prog.running = 1;
        viewer->callbackData = &prog;
        viewer->resize_callback = resize_callback;
        viewer->key_callback = key_callback;
        viewer->close_callback = close_callback;
        scene_update_nodes(&prog.scene, NULL, NULL);
        for (i = 0; i < prog.metadata.numLightNodes; i++) {
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
                case NODE_SLIGHT:
                    if (prog.numSpotLights < MAX_SPOT_LIGHTS) {
                        lights_buffer_object_update_slight(&prog.scene.lights, n->data.slight, prog.numSpotLights);
                        prog.slights[prog.numSpotLights++] = n;
                    } else {
                        fprintf(stderr, "Warning: point lights limit exceeded\n");
                    }
                    break;
                default:;
            }
        }
        for (i = 0; i < prog.metadata.numClips; i++) {
            prog.metadata.clips[i]->loop = 1;
            prog.metadata.clips[i]->mode = CLIP_FORWARD;
            printf("Found Clip #%d", i);
            if (prog.metadata.clips[i]->name) {
                printf(" (%s)", prog.metadata.clips[i]->name);
            }
            printf("\n");
        }
        {
            struct AmbientLight ambient = {0};
            lights_buffer_object_update_ambient(&prog.scene.lights, &ambient);
        }
        lights_buffer_object_update_ndlight(&prog.scene.lights, prog.numDirectionalLights);
        lights_buffer_object_update_nplight(&prog.scene.lights, prog.numPointLights);
        lights_buffer_object_update_nslight(&prog.scene.lights, prog.numSpotLights);
        camera = prog.metadata.cameraNodes[0]->data.camera;
        camera_set_ratio(((float)viewer->width) / ((float)viewer->height), camera->projection);
        camera_buffer_object_update_projection(&prog.scene.camera, MAT_CONST_CAST(camera->projection));
        camera_buffer_object_update_view_and_position(&prog.scene.camera, MAT_CONST_CAST(camera->view));
        uniform_buffer_send(&prog.scene.lights);
        uniform_buffer_send(&prog.scene.camera);
        glfwSwapInterval(1);
        while (prog.running) {
            viewer_process_events(viewer);
            dt = viewer_next_frame(viewer);
            if (prog.metadata.numClips) {
                if (prog.metadata.numClips) {
                    anim_play_clip(prog.metadata.clips[prog.activeClip], dt);
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
    if (sceneInit) {
        for (i = 0; i < prog.scene.root.nbChildren; i++) {
            nodes_free(prog.scene.root.children[i], imported_node_free);
        }
        prog.scene.root.nbChildren = 0;
        scene_free(&prog.scene, NULL);
    }
    if (ogexInit) {
        import_free_shared_data(&shared);
        import_free_metadata(&prog.metadata);
    }
    if (viewer) viewer_free(viewer);
    game_free();

    return err;
}
