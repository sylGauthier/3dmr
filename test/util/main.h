#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <game/asset_manager.h>
#include <game/render/viewer.h>
#include <game/scene/scene.h>

#include "callbacks.h"

int run(struct Viewer* viewer, struct Scene* scene, int argc, char** argv);

int main(int argc, char** argv) {
    struct Scene scene;
    struct Viewer* viewer = NULL;
    char *ptr, *tmp, *title;
    const char* screenshot = NULL;
    unsigned int width = 640, height = 480;
    int timeout = -1;
    time_t start, t;
    int ret = 0;

    if (!(title = strrchr(argv[0], '/'))) {
        title = argv[0];
    }
    if ((ptr = strstr(argv[0], "test/")) && !strchr(ptr + 5, '/')) {
        if (!(tmp = malloc((ptr - argv[0]) + 12))) {
            fprintf(stderr, "Error: memory allocation failed");
            return 1;
        }
        memcpy(tmp, argv[0], (ptr - argv[0]) + 5);
        memcpy(tmp + (ptr - argv[0]) + 5, "assets", 7);
        asset_manager_add_path(tmp);
        tmp[ptr - argv[0]] = 0;
        asset_manager_add_path(tmp);
        free(tmp);
    } else if (argv[0][0] == '.' && argv[0][1] == '/' && !strchr(argv[0] + 2, '/')) {
        asset_manager_add_path("assets");
        asset_manager_add_path("..");
    } else {
        fprintf(stderr, "Error: unable to figure out asset paths\n");
        return 1;
    }

    argv++; argc--;
    while (argc && **argv == '-') {
        if ((*argv)[1] == '-') {
            if (!(*argv)[2]) {
                argc--;
                argv++;
            }
            break;
        }
        argc--;
        for (ptr = (*argv) + 1; *ptr; ptr++) {
            switch (*ptr) {
                case 'd':
                    if (argc >= 2) {
                        width = strtoul(*++argv, NULL, 10);
                        height = strtoul(*++argv, NULL, 10);
                        argc -= 2;
                    } else {
                        fprintf(stderr, "Error: -d needs two parameters (width, height)\n");
                        return 1;
                    }
                    break;
                case 's':
                    if (argc) {
                        screenshot = *++argv;
                        argc--;
                    } else {
                        fprintf(stderr, "Error: -s needs one parameter (screenshot path)\n");
                        return 1;
                    }
                    break;
                case 't':
                    if (argc) {
                        timeout = strtol(*++argv, NULL, 10);
                        argc--;
                    } else {
                        fprintf(stderr, "Error: -t needs one parameter (delay)\n");
                        return 1;
                    }
                    break;
                default:
                    fprintf(stderr, "Error: unrecognised option: %c\n", *ptr);
                    return 1;
            }
        }
        argv++;
    }

    if (!(viewer = viewer_new(width, height, title))) {
        fprintf(stderr, "Error: cannot start viewer\n");
        return 1;
    }
    scene_init(&scene);
    if (run(viewer, &scene, argc, argv)) {
        ret = 1;
    } else {
        viewer->key_callback = key_callback;
        viewer->close_callback = close_callback;
        running = 1;
        for (start = t = time(NULL); running && (timeout < 0 || t < start + timeout); t = time(NULL)) {
            usleep(10 * 1000);
            viewer_process_events(viewer);
            viewer_next_frame(viewer);
            scene_render(&scene, &viewer->camera);
        }
        if (screenshot) {
            if (!viewer_screenshot(viewer, screenshot)) {
                fprintf(stderr, "Error: failed to take screenshot\n");
                ret = 1;
            }
        }
    }
    scene_free(&scene);
    viewer_free(viewer);
    asset_manager_free();

    return ret;
}
