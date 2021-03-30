#include <stdlib.h>
#include <string.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <3dmr/shaders.h>
#include <3dmr/animation/play.h>
#include <3dmr/render/camera_buffer_object.h>
#include <3dmr/render/lights_buffer_object.h>
#include <3dmr/scene/opengex.h>
#include <3dmr/scene/gltf.h>
#include <3dmr/skybox.h>

struct Prog {
    struct Scene scene;
    struct IBL ibl;
    struct ImportMetadata metadata;
    unsigned int activeCamIdx, activeClip, numDirectionalLights, numPointLights, numSpotLights;
    struct Node *dlights[MAX_DIRECTIONAL_LIGHTS], *plights[MAX_POINT_LIGHTS], *slights[MAX_SPOT_LIGHTS];
    struct Node *defCam, *defCamPivot, *activeCam;
    int running;
};

static void usage(const char* prog) {
    printf("Usage: %s [--ibl <file>] file [file2 [file3 ...]]\n", prog);
}

static void resize_callback(struct Viewer* viewer, void* data) {
    struct Prog* prog = data;
    struct Camera* activeCam = prog->activeCam->data.camera;
    glViewport(0, 0, viewer->width, viewer->height);
    camera_set_ratio(((float)viewer->width) / ((float)viewer->height), activeCam->projection);
    camera_buffer_object_update_projection(&prog->scene.camera, MAT_CONST_CAST(activeCam->projection));
    uniform_buffer_send(&prog->scene.camera);
}

static void update_cam(struct Viewer* viewer, struct Prog* prog) {
    struct Camera* activeCam;
    activeCam = prog->activeCam->data.camera;
    camera_set_ratio(((float)viewer->width) / ((float)viewer->height), activeCam->projection);
    camera_buffer_object_update_projection(&prog->scene.camera, MAT_CONST_CAST(activeCam->projection));
    camera_buffer_object_update_view_and_position(&prog->scene.camera, MAT_CONST_CAST(activeCam->view));
    uniform_buffer_send(&prog->scene.camera);
    if (prog->activeCam->name) {
        printf("Current camera: %s\n", prog->activeCam->name);
    } else {
        printf("Current camera: #%u\n", prog->activeCamIdx);
    }
}

static void cursor_callback(struct Viewer* viewer, double xpos, double ypos,
                            double dx, double dy, int bl, int bm, int br,
                            void* data) {
    struct Prog* prog = data;
    Vec3 axisX = {1, 0, 0};
    Vec3 axisY = {0, 1, 0};

    if (br && prog->activeCam == prog->defCam) {
        node_rotate(prog->defCamPivot, axisY, -dx / viewer->width);
        node_slew(prog->defCamPivot, axisX, -dy / viewer->width);
    }
    return;
}

static void wheel_callback(struct Viewer* viewer, double dx, double dy, void* data) {
    struct Prog* prog = data;

    if (prog->activeCam == prog->defCam) {
        Vec3 t;
        t[0] = 0;
        t[1] = 0;
        if (dy < 0)
            t[2] = prog->activeCam->position[2] * (1 - dy / 10.);
        else
            t[2] = prog->activeCam->position[2] / (1 + dy / 10.);
        node_set_pos(prog->activeCam, t);
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
            if (prog->metadata.numCameraNodes) {
                prog->activeCamIdx = (prog->activeCamIdx + prog->metadata.numCameraNodes - 1) % prog->metadata.numCameraNodes;
                prog->activeCam = prog->metadata.cameraNodes[prog->activeCamIdx];
                update_cam(viewer, prog);
            }
            break;
        case GLFW_KEY_RIGHT: case GLFW_KEY_DOWN:
            if (prog->metadata.numCameraNodes) {
                prog->activeCamIdx = (prog->activeCamIdx + 1) % prog->metadata.numCameraNodes;
                prog->activeCam = prog->metadata.cameraNodes[prog->activeCamIdx];
                update_cam(viewer, prog);
            }
            break;
        case GLFW_KEY_HOME:
            prog->activeCam = prog->defCam;
            update_cam(viewer, prog);
            break;
        case GLFW_KEY_SPACE:
            if (!prog->metadata.numClips) {
                printf("No clip.\n");
                return;
            }
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
            if (n == prog->activeCam) {
                camera_buffer_object_update_view_and_position(&scene->camera, MAT_CONST_CAST(n->data.camera->view));
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
    FILE* file = NULL;
    unsigned int i, ok = 1;

    for (i = 0; i < argc && ok; i++) {
        char* extension;
        printf("Appending scene: %s\n", argv[i]);
        if (!(extension = strrchr(argv[i], '.'))) {
            fprintf(stderr, "Error: no file extension\n");
            ok = 0;
        } else if (!(file = fopen(argv[i], "r"))) {
            fprintf(stderr, "Error: could not open file: %s\n", argv[i]);
            ok = 0;
        } else if (!strcmp(extension, ".ogex")) {
#ifdef TDMR_OPENGEX
            if (!ogex_load(&prog->scene.root, file, dirname(argv[i]), shared, &prog->metadata)) {
                fprintf(stderr, "Error: failed to load ogex file\n");
                ok = 0;
            }
#else
            fprintf(stderr, "Warning: 3dmrview was built without opengex support, skipping ogex file\n");
#endif
        } else if (!strcmp(extension, ".gltf") || !strcmp(extension, ".glb")) {
#ifdef TDMR_GLTF
            struct ImportOptions opts = {0};
            if (prog->ibl.enabled) opts.ibl = &prog->ibl;
            opts.binary = !strcmp(extension, ".glb");
            if (!gltf_load(&prog->scene.root, file, dirname(argv[i]), shared, &prog->metadata, &opts)) {
                fprintf(stderr, "Error: failed to load gltf file\n");
                ok = 0;
            }
#else
            fprintf(stderr, "Warning: 3dmrview was built without gltf support, skipping gltf file\n");
#endif
        } else {
            fprintf(stderr, "Error: invalid extension: %s\n", extension);
            ok = 0;
        }
        if (file) fclose(file);
    }
    if (!ok) {
        import_free_shared_data(shared);
        import_free_metadata(&prog->metadata);
    }
    return ok;
}

static int init_def_cam(struct Prog* prog) {
    struct Camera* cam = NULL;

    prog->defCam = NULL;
    prog->defCamPivot = NULL;
    if (       !(prog->defCam = malloc(sizeof(struct Node)))
            || !(prog->defCamPivot = malloc(sizeof(struct Node)))
            || !(cam = malloc(sizeof(struct Camera)))) {
        fprintf(stderr, "Error: can allocate default cam\n");
    } else if (node_init(prog->defCam), node_init(prog->defCamPivot), !node_add_child(prog->defCamPivot, prog->defCam)) {
        fprintf(stderr, "Error: can't init default cam\n");
    } else if (!scene_add(&prog->scene, prog->defCamPivot)) {
        fprintf(stderr, "Error: can't add default cam to scene\n");
    } else {
        Vec3 t = {0., 0., 5.}, axisX = {1, 0, 0}, axisY = {0, 1, 0};

        node_rotate(prog->defCamPivot, axisY, M_PI / 4.);
        node_slew(prog->defCamPivot, axisX, -M_PI / 4.);
        node_translate(prog->defCam, t);

        node_set_name(prog->defCam, "default");
        node_set_camera(prog->defCam, cam);

        load_id4(cam->view);
        camera_projection(1., 60 / 360. * 2 * M_PI, 0.001, 1000., cam->projection);
        return 1;
    }
    free(cam);
    free(prog->defCamPivot->children);
    free(prog->defCamPivot);
    free(prog->defCam);
    return 0;
}

static int load_ibl(struct Prog* prog, const char* filename) {
    GLuint tex;

    if (!(tex = skybox_load_texture_hdr_equirect(filename, 1024))) {
        fprintf(stderr, "Error: can't load ibl texture\n");
        return 0;
    }
    return compute_ibl(tex, 32, 1024, 5, 256, &prog->ibl);
}

int main(int argc, char** argv) {
    struct Prog prog;
    struct ImportSharedData shared;
    struct Viewer* viewer = NULL;
    struct Camera* camera;
    unsigned int i;
    int sceneInit = 0, ogexInit = 0, camInit = 0, err = 1;
    double dt;

    if (argc < 2) {
        usage(argv[0]);
        return 1;
    }
#ifdef TDMR_SHADERS_PATH
    tdmrShaderRootPath = TDMR_SHADERS_PATH;
#endif

    prog.numDirectionalLights = 0;
    prog.numPointLights = 0;
    prog.numSpotLights = 0;
    import_init_metadata(&prog.metadata);
    import_init_shared_data(&shared);

    if (!(viewer = viewer_new(1024, 768, argv[1]))) {
        fprintf(stderr, "Error: failed to start viewer\n");
        goto exit;
    }

    if (!strcmp(argv[1], "--ibl")) {
        if (argc < 4) {
            usage(argv[0]);
            goto exit;
        }
        if (!load_ibl(&prog, argv[2])) {
            fprintf(stderr, "Error: loading IBL failed\n");
            goto exit;
        }
        prog.ibl.enabled = 1;
        argc -= 2;
        argv += 2;
    }

    if (!(sceneInit = scene_init(&prog.scene, NULL))) {
        fprintf(stderr, "Error: failed to init scene\n");
    } else if (!(ogexInit = load_scenes(&prog, &shared, argc - 1, argv + 1))) {
        fprintf(stderr, "Error: failed to load scenes\n");
    } else if (!(camInit = init_def_cam(&prog))) {
        fprintf(stderr, "Error: failed to init default camera\n");
    } else {
        err = 0;
        prog.activeCamIdx = 0;
        prog.activeCam = prog.defCam;
        prog.activeClip = 0;
        prog.running = 1;
        viewer->callbackData = &prog;
        viewer->resize_callback = resize_callback;
        viewer->key_callback = key_callback;
        viewer->cursor_callback = cursor_callback;
        viewer->wheel_callback = wheel_callback;
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
        camera = prog.activeCam->data.camera;
        camera_set_ratio(((float)viewer->width) / ((float)viewer->height), camera->projection);
        camera_buffer_object_update_projection(&prog.scene.camera, MAT_CONST_CAST(camera->projection));
        camera_buffer_object_update_view_and_position(&prog.scene.camera, MAT_CONST_CAST(camera->view));
        uniform_buffer_send(&prog.scene.lights);
        uniform_buffer_send(&prog.scene.camera);
        glfwSwapInterval(1);
        glClearColor(0.2, 0.2, 0.2, 0);
        while (prog.running) {
            dt = viewer_next_frame(viewer);
            viewer_process_events(viewer);
            if (prog.metadata.numClips) {
                anim_play_clip(prog.metadata.clips[prog.activeClip], dt);
            }
            scene_update_nodes(&prog.scene, update_node, &prog);
            uniform_buffer_send(&prog.scene.lights);
            uniform_buffer_send(&prog.scene.camera);
            camera = prog.activeCam->data.camera;
            scene_update_render_queue(&prog.scene, MAT_CONST_CAST(camera->view), MAT_CONST_CAST(camera->projection));
            scene_render(&prog.scene);
        }
    }

exit:
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

    return err;
}
