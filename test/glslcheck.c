#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/glew.h>
#include <game/asset_manager.h>
#include <game/render/shader.h>
#include <game/render/viewer.h>

static void usage(const char* prog) {
    printf("Usage: %s [-I dir]... [-D macro[=value]]... [-S vert|tesc|tese|geom|frag|comp] [file]\n", prog);
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

int add_define(char*** defines, size_t* numDefines, const char* macro) {
    char** new;
    const char *ptr, *end;
    size_t n = *numDefines;

    if (n == ((size_t)-1)
     || !(new = realloc(*defines, (2 * ++n) * sizeof(**defines)))) {
        return 0;
    }
    *defines = new;
    new += 2 * (n - 1);
    new[1] = NULL;
    for (ptr = macro; *ptr && *ptr != '='; ptr++);
    for (end = ptr; *end; end++);
    if (!(new[0] = malloc((ptr - macro) + 1))
     || (*ptr == '=' && !(new[1] = malloc(end - ptr)))) {
        free(new[0]);
        return 0;
    }
    memcpy(new[0], macro, ptr - macro);
    new[0][ptr - macro] = 0;
    if (new[1]) {
        ptr++;
        memcpy(new[1], ptr, end - ptr);
        new[1][end - ptr] = 0;
    }
    *numDefines = n;
    return 1;
}

void free_defines(char** defines, size_t numDefines) {
    while (numDefines) {
        free(defines[2 * --numDefines + 1]);
        free(defines[2 * numDefines]);
    }
    free(defines);
}

int main(int argc, char** argv) {
    struct Viewer* viewer;
    char** defines = NULL;
    const char* stage = NULL;
    const char* path = NULL;
    FILE* in = NULL;
    GLenum type = 0;
    GLuint shader;
    size_t numDefines = 0;
    int i, ret = 0;
    unsigned int j;

    for (i = 1; !ret && i < argc; i++) {
        if (argv[i][0] == '-') {
            switch (argv[i][1]) {
                case 'I':
                    if (argv[i][2]) {
                        asset_manager_add_path(argv[i] + 2);
                    } else if (++i < argc) {
                        asset_manager_add_path(argv[i]);
                    } else {
                        ret = 1;
                    }
                    continue;

                case 'D':
                    if (argv[i][2]) {
                        ret = 2 * !add_define(&defines, &numDefines, argv[i] + 2);
                    } else if (++i < argc) {
                        ret = 2 * !add_define(&defines, &numDefines, argv[i]);
                    } else {
                        ret = 1;
                    }
                    continue;

                case 'S':
                    if (argv[i][2]) {
                        stage = argv[i] + 2;
                    } else if (++i < argc) {
                        stage = argv[i];
                    } else {
                        ret = 1;
                    }
                    for (j = 0; j < sizeof(stages) / sizeof(*stages); j++) {
                        if (!strcmp(stage, stages[j].name)) {
                            type = stages[j].type;
                            break;
                        }
                    }
                    if (j == sizeof(stages) / sizeof(*stages)) {
                        fprintf(stderr, "Error: invalid stage string\n");
                        ret = 1;
                    }
                    continue;

                case '-':
                    i++;
                    break;

                case 'h':
                    usage(argv[0]);
                    free_defines(defines, numDefines);
                    return 0;

                default:
                    ret = 1;
            }
        }
        break;
    }
    if (ret) {
        if (ret == 1) {
            usage(argv[0]);
        } else if (ret == 2) {
            fprintf(stderr, "Error: memory allocation failed\n");
            ret = 1;
        }
    } else if (i == argc - 1) {
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
                ret = 1;
            }
        }
        if (!ret && !(in = fopen(path, "r"))) {
            fprintf(stderr, "Error: unable to open '%s'\n", path);
            ret = 1;
        }
    } else if (i == argc) {
        path = "<stdin>";
        in = stdin;
        if (!stage) {
            fprintf(stderr, "Error: when stdin is used, -S must be specified\n");
            ret = 1;
        }
    } else {
        usage(argv[0]);
        ret = 1;
    }
    if (!ret) {
        if ((viewer = viewer_new(640, 480, ""))) {
            if ((shader = shader_compile_fd(in, path, type, (const char**)defines, numDefines))) {
                glDeleteShader(shader);
            } else {
                ret = 1;
            }
            viewer_free(viewer);
        }
    }
    if (i == argc - 1 && in) {
        fclose(in);
    }
    free_defines(defines, numDefines);
    return ret;
}
