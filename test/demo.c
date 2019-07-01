#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <game/init.h>
#include <game/skybox.h>
#include <game/render/viewer.h>
#include <game/render/lights_buffer_object.h>

#include "test/demo_util/args.h"
#include "test/demo_util/asset_manager.h"
#include "test/demo_util/callbacks.h"
#include "test/demo_util/objects.h"
#include "test/demo_util/materials.h"
#include "test/demo_util/object_args.h"
#include "test/demo_util/paths.h"

static void usage(const char* progname) {
    printf("Usage: %s [viewer/light args]* [object;material[;object arg]*]*\n\n", progname);
    usage_viewer();
    usage_objects();
    usage_materials();
    usage_object_args();
}

int demo_scene_init(struct Config* config, struct Viewer* viewer) {
    struct Lights lights;
    lights = config->scene.lights;
    if (scene_init(&config->scene, &viewer->camera)) {
        config->scene.lights = lights;
        lights_buffer_object_update(&lights, config->scene.uboLights);
        return 1;
    }
    return 0;
}

int main(int argc, char** argv) {
    char* title = NULL;
    struct Config config;
    struct Viewer* viewer = NULL;
    double dt, t = 0, ct = 0, cf = 0, fps = 0;
    int ret = 0;

    if (!register_asset_paths(argv[0])) return 1;
    if (!game_init(asset_manager_get_path(1))) {
        asset_manager_free();
        return 1;
    }

    config_init(&config);
    if (!parse_args(&argc, &argv, &config, usage, 1)) {
        ret = 1;
    } else if (!(viewer = viewer_new(config.width, config.height, config.title))) {
        fprintf(stderr, "Error: cannot start viewer\n");
        ret = 1;
    } else if (!demo_scene_init(&config, viewer)) {
        fprintf(stderr, "Error: failed to init scene\n");
        ret = 1;
    } else if (!parse_args_objects(&argc, &argv, &config)) {
        ret = 1;
    } else {
        viewer->key_callback = key_callback;
        viewer->cursor_callback = cursor_rotate_camera;
        viewer->wheel_callback = wheel_callback;
        viewer->close_callback = close_callback;
        glfwSwapInterval(1);
        running = 1;
        title = malloc(strlen(config.title) + 128);
        for (t = 0; running && (config.timeout < 0 || t < config.timeout); t += dt) {
            unsigned int nb;
            viewer_process_events(viewer);
            dt = viewer_next_frame(viewer);
            if (config.skybox) {
                skybox_render(config.skybox);
            }
            nb = scene_render_count(&config.scene);
            update_materials(dt);
            if (title) {
                ct += dt;
                cf++;
                if (ct > 1.0) {
                    fps = cf / ct;
                    ct = cf = 0;
                    sprintf(title, "%s - %u FPS - %d rendered obj", config.title, (unsigned int)fps, nb);
                    viewer_set_title(viewer, title);
                }
            }
        }
        if (config.screenshot) {
            if (!viewer_screenshot(viewer, config.screenshot)) {
                fprintf(stderr, "Error: failed to take screenshot\n");
                ret = 1;
            }
        }
    }

    viewer_free(viewer);
    config_free(&config);
    free(title);
    asset_manager_free();
    game_free();
    return ret;
}
