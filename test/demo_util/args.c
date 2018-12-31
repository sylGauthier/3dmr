#include <stdio.h>
#include <stdlib.h>
#include <game/skybox.h>
#include "args.h"
#include "lights.h"
#include "materials.h"
#include "objects.h"

void usage_viewer(void) {
    printf(
            "Viewer arguments:\n"
            "  -d width height                      define the viewer size\n"
            "  -t timeout                           exit after a timeout\n"
            "  -s screenshotPath                    save a screenshot\n"
            "\n"
            "Light arguments:\n"
            "  -La color                            define the ambient light\n"
            "  -Lp position;color;radius            define a point light\n"
            "  -Ld direction;color                  define a directional light\n"
            "\n"
          );
}

void config_init(struct Config* config) {
    scene_init(&config->scene);
    config->skybox = NULL;
    config->objects = NULL;
    config->numObjects = 0;
    config->screenshot = NULL;
    config->title = "demo";
    config->width = 640;
    config->height = 480;
    config->timeout = -1;
}

void config_free(struct Config* config) {
    while (config->numObjects) {
        config_object_free(config, config->numObjects - 1);
    }
    skybox_free(config->skybox);
    scene_free(&config->scene);
    config->scene.root.nbChildren = 0;
    free(config->objects);
}

void config_object_free(struct Config* config, unsigned int i) {
    struct GLObject* object;
    unsigned int j;

    if (i < config->numObjects) {
        if (config->objects[i]->nbChildren) { /*grid*/
            object = config->objects[i]->children[0]->object;
        } else {
            object = config->objects[i]->object;
        }
        for (j = 0; j < config->scene.root.nbChildren; j++) {
            if (config->scene.root.children[j] == config->objects[i]) {
                config->scene.root.children[j] = config->scene.root.children[--config->scene.root.nbChildren];
                break;
            }
        }
        vertex_array_free(object->vertexArray);
        free(object->material);
        free(object);
        graph_free(config->objects[i]);
        free(config->objects[i]);
        config->objects[i] = config->objects[--config->numObjects];
    }
}

static int parse_vecN(const char* s, char** e, float* out, unsigned int n) {
    char* end;
    unsigned int i;

    for (i = 0; i < n; i++) {
        out[i] = strtod(s, &end);
        if (end == s || (i < (n - 1) && *end++ != ',')) {
            return 0;
        }
        s = end;
    }
    if (e) {
        *e = end;
    }
    return 1;
}

int parse_float(const char* s, char** e, float* out) {
    return parse_vecN(s, e, out, 1);
}

int parse_vec2(const char* s, char** e, Vec2 out) {
    return parse_vecN(s, e, out, 2);
}

int parse_vec3(const char* s, char** e, Vec3 out) {
    return parse_vecN(s, e, out, 3);
}

int parse_vec4(const char* s, char** e, Vec4 out) {
    return parse_vecN(s, e, out, 4);
}

int parse_args(int* argc, char*** argv, struct Config* config, void (*usage)(const char*), int warnNoArgs) {
    const char* progname;
    const char* ptr;

    progname = *((*argv)++);
    (*argc)--;
    while (*argc && ***argv == '-') {
        if ((**argv)[1] == '-') {
            if (!(**argv)[2]) {
                (*argc)--;
                (*argv)++;
            }
            break;
        }
        (*argc)--;
        for (ptr = (**argv) + 1; *ptr; ptr++) {
            switch (*ptr) {
                case 'h':
                    usage(progname);
                    break;
                case 'd':
                    if (*argc >= 2) {
                        config->width = strtoul(*++(*argv), NULL, 10);
                        config->height = strtoul(*++(*argv), NULL, 10);
                        (*argc) -= 2;
                    } else {
                        fprintf(stderr, "Error: -d needs two parameters (width, height)\n");
                        return 0;
                    }
                    break;
                case 's':
                    if (*argc) {
                        config->screenshot = *++(*argv);
                        (*argc)--;
                    } else {
                        fprintf(stderr, "Error: -s needs one parameter (screenshot path)\n");
                        return 0;
                    }
                    break;
                case 'T':
                    if (*argc) {
                        config->title = *++(*argv);
                        (*argc)--;
                    } else {
                        fprintf(stderr, "Error: -T needs one parameter (title)\n");
                        return 0;
                    }
                    break;
                case 't':
                    if (*argc) {
                        config->timeout = strtol(*++(*argv), NULL, 10);
                        (*argc)--;
                    } else {
                        fprintf(stderr, "Error: -t needs one parameter (delay)\n");
                        return 0;
                    }
                    break;
                case 'L':
                    switch (*++ptr) {
                        case 'a':
                            if (!*argc || !parse_ambient_light(*++(*argv), &config->scene.lights.ambientLight)) {
                                fprintf(stderr, "Error: wrong -La parameter, see usage\n");
                                return 0;
                            }
                            (*argc)--;
                            break;
                        case 'p':
                            if (config->scene.lights.numPointLights >= MAX_POINT_LIGHTS) {
                                fprintf(stderr, "Error: too much point lights\n");
                                return 0;
                            }
                            if (!*argc || !parse_point_light(*++(*argv), config->scene.lights.point + config->scene.lights.numPointLights++)) {
                                fprintf(stderr, "Error: wrong -Lp parameter, see usage\n");
                                return 0;
                            }
                            (*argc)--;
                            break;
                        case 'd':
                            if (config->scene.lights.numDirectionalLights >= MAX_DIRECTIONAL_LIGHTS) {
                                fprintf(stderr, "Error: too much directional lights\n");
                                return 0;
                            }
                            if (!*argc || !parse_directional_light(*++(*argv), config->scene.lights.directional + config->scene.lights.numDirectionalLights++)) {
                                fprintf(stderr, "Error: wrong -Ld parameter, see usage\n");
                                return 0;
                            }
                            (*argc)--;
                            break;
                        default:
                            fprintf(stderr, "Error: unknown light type, see usage\n");
                            return 0;
                    }
                    break;
                default:
                    fprintf(stderr, "Error: unrecognised option: %c\n", *ptr);
                    return 0;
            }
        }
        (*argv)++;
    }

    if (!*argc && warnNoArgs) {
        printf("Use '%s -h' to get program usage.\n", progname);
    }
    return 1;
}

int parse_args_objects(int* argc, char*** argv, struct Config* config) {
    while ((*argc)--) {
        if (!parse_object(*(*argv)++, config)) {
            return 0;
        }
    }
    return 1;
}
