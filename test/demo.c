#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include <game/asset_manager.h>
#include <game/skybox.h>
#include <game/render/viewer.h>

#include "test/demo_util/args.h"
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

int main(int argc, char** argv) {
    struct Config config;
    struct Viewer* viewer = NULL;
    time_t start, t;
    double dt;
    int ret = 0;

    if (!register_asset_paths(argv[0])) return 1;

    config_init(&config);
    if (!parse_args(&argc, &argv, &config, usage, 1)) {
        ret = 1;
    } else if (!(viewer = viewer_new(config.width, config.height, config.title))) {
        fprintf(stderr, "Error: cannot start viewer\n");
        ret = 1;
    } else if (!parse_args_objects(&argc, &argv, &config)) {
        ret = 1;
    } else {
        viewer->key_callback = key_callback;
        viewer->cursor_callback = cursor_rotate_camera;
        viewer->wheel_callback = wheel_callback;
        viewer->close_callback = close_callback;
        running = 1;
        for (start = t = time(NULL); running && (config.timeout < 0 || t < start + config.timeout); t = time(NULL)) {
            usleep(10 * 1000);
            viewer_process_events(viewer);
            dt = viewer_next_frame(viewer);
            if (config.skybox) {
                skybox_render(config.skybox, &viewer->camera);
            }
            scene_render(&config.scene, &viewer->camera);
            update_materials(dt);
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
    asset_manager_free();
    return ret;
}
