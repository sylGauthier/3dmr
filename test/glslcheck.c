#include <stdio.h>
#include <string.h>
#include <GL/glew.h>
#include <game/asset_manager.h>
#include <game/render/shader.h>
#include <game/render/viewer.h>

static void usage(const char* prog) {
    printf("Usage: %s [-I dir]... [-S vert|tesc|tese|geom|frag|comp] [file]\n", prog);
}

struct Stage {
    const char name[5];
    GLenum type;
} stages[] = {
    {"vert", GL_VERTEX_SHADER},
    {"tesc", GL_TESS_CONTROL_SHADER},
    {"tese", GL_TESS_EVALUATION_SHADER},
    {"geom", GL_GEOMETRY_SHADER},
    {"frag", GL_FRAGMENT_SHADER},
    {"comp", GL_COMPUTE_SHADER}
};

int main(int argc, char** argv) {
    struct Viewer* viewer;
    char* stage = NULL;
    char* path = NULL;
    FILE* in = NULL;
    GLenum type = 0;
    int i, ret = 0;
    unsigned int j;

    for (i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            switch (argv[i][1]) {
                case 'I':
                    if (argv[i][2]) {
                        asset_manager_add_path(argv[i] + 2);
                    } else if (++i < argc) {
                        asset_manager_add_path(argv[i]);
                    } else {
                        usage(argv[0]);
                        return 1;
                    }
                    continue;

                case 'S':
                    if (argv[i][2]) {
                        stage = argv[i] + 2;
                    } else if (++i < argc) {
                        stage = argv[i];
                    } else {
                        usage(argv[0]);
                        return 1;
                    }
                    for (j = 0; j < sizeof(stages) / sizeof(*stages); j++) {
                        if (!strcmp(stage, stages[j].name)) {
                            type = stages[j].type;
                            break;
                        }
                    }
                    if (j == sizeof(stages) / sizeof(*stages)) {
                        fprintf(stderr, "Error: invalid stage string\n");
                        return 1;
                    }
                    continue;

                case '-':
                    i++;
                    break;

                case 'h':
                    usage(argv[0]);
                    return 0;

                default:
                    usage(argv[0]);
                    return 1;
            }
        }
        break;
    }
    if (i == argc - 1) {
        path = argv[i];
        if (!stage) {
            for (stage = path; *stage; stage++) {
                if (*stage == '.') {
                    for (j = 0; j < sizeof(stages) / sizeof(*stages); j++) {
                        if (!memcmp(stage + 1, stages[j].name, 4) && (!stage[5] || !strcmp(stage + 5, ".glsl"))) {
                            stage++;
                            type = stages[j].type;
                            break;
                        }
                    }
                    if (*stage != '.') {
                        break;
                    }
                }
            }
            if (!*stage) {
                fprintf(stderr, "Error: cannot guess shader type from extension\n");
                return 1;
            }
        }
        if (!(in = fopen(path, "r"))) {
            fprintf(stderr, "Error: unable to open '%s'\n", path);
            return 1;
        }
    } else if (i == argc) {
        path = "<stdin>";
        in = stdin;
        if (!stage) {
            fprintf(stderr, "Error: when stdin is used, -S must be specified\n");
            return 1;
        }
    } else {
        usage(argv[0]);
        return 1;
    }
    if ((viewer = viewer_new(640, 480, ""))) {
        if ((ret = shader_compile_fd(in, path, type))) {
            glDeleteShader(ret);
        }
    }
    if (i == argc - 1) {
        fclose(in);
    }
    return !ret;
}
