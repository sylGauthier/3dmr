#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <game/init.h>
#include <game/render/shader.h>
#include <game/render/viewer.h>
#include <game/render/camera_buffer_object.h>
#include <game/render/lights_buffer_object.h>
#include "src/material/programs.h"
#include "ubo.h"

int test_ubo(GLuint prog, GLuint ubo, const char* uboName, unsigned long (*get_offset)(const char*)) {
    char* name;
    unsigned long offset, expected;
    GLuint i, id, numUniforms;
    GLint tmp;
    int ret = 1;

    id = glGetUniformBlockIndex(prog, uboName);
    if (id == GL_INVALID_INDEX) return 0;
    glGetProgramiv(prog, GL_ACTIVE_UNIFORMS, &tmp);
    if (tmp < 0) return 0;
    numUniforms = tmp;
    for (i = 0; i < numUniforms; i++) {
        glGetActiveUniformsiv(prog, 1, &i, GL_UNIFORM_BLOCK_INDEX, &tmp);
        if (tmp < 0 || ((GLuint)tmp) != id) continue;
        glGetActiveUniformsiv(prog, 1, &i, GL_UNIFORM_NAME_LENGTH, &tmp);
        if (tmp < 0 || !(name = malloc(((GLuint)tmp) + 1))) return 0;
        glGetActiveUniformName(prog, i, ((GLuint)tmp) + 1, NULL, name);
        name[tmp] = 0;
        offset = get_offset(name);
        glGetActiveUniformsiv(prog, 1, &i, GL_UNIFORM_OFFSET, &tmp);
        if (tmp < 0) {
            free(name);
            return 0;
        }
        expected = tmp;
        printf("%c %-44s %016lX %016lX\n", (offset == expected) ? ' ' : '>', name, offset, expected);
        ret = ret && (offset == expected);
        free(name);
    }
    return ret;
}

unsigned long camera_offset(const char* name) {
    if (!strcmp(name, "projection")) {
        return OFFSET_PROJECTION;
    } else if (!strcmp(name, "view")) {
        return OFFSET_VIEW;
    } else if (!strcmp(name, "cameraPosition")) {
        return OFFSET_POSITION;
    }
    return -1;
}

int test_camera(GLuint prog) {
    struct UniformBuffer u;
    int ret;

    if (!camera_buffer_object_gen(&u)) return 0;
    ret = test_ubo(prog, u.ubo, "Camera", camera_offset);
    uniform_buffer_del(&u);
    return ret;
}

unsigned long lights_offset(const char* name) {
    unsigned long i;

    if (!strcmp(name, "ambientLight.color")) {
        return OFFSET_AMBIENT;
    } else if (!strncmp(name, "directionalLights[", 18)) {
        i = strtoul(name + 18, (char**)&name, 10);
        if (*name++ != ']') return -1;
        if (*name++ != '.') return -1;
        if (!strcmp(name, "direction")) {
            return OFFSET_DLIGHT_DIRECTION(i);
        } else if (!strcmp(name, "color")) {
            return OFFSET_DLIGHT_COLOR(i);
        }
    } else if (!strncmp(name, "pointLights[", 12)) {
        i = strtoul(name + 12, (char**)&name, 10);
        if (*name++ != ']') return -1;
        if (*name++ != '.') return -1;
        if (!strcmp(name, "position")) {
            return OFFSET_PLIGHT_POSITION(i);
        } else if (!strcmp(name, "color")) {
            return OFFSET_PLIGHT_COLOR(i);
        } else if (!strcmp(name, "radius")) {
            return OFFSET_PLIGHT_RADIUS(i);
        }
    } else if (!strcmp(name, "numDirectionalLights")) {
        return OFFSET_NUM_DLIGHTS;
    } else if (!strcmp(name, "numPointLights")) {
        return OFFSET_NUM_PLIGHTS;
    }
    return -1;
}

int test_lights(GLuint prog) {
    struct UniformBuffer u;
    int ret;

    if (!lights_buffer_object_gen(&u)) return 0;
    ret = test_ubo(prog, u.ubo, "Lights", lights_offset);
    uniform_buffer_del(&u);
    return ret;
}

int main() {
    static const char* defines[] = {"HAVE_NORMAL", NULL};
    struct Viewer* viewer;
    GLuint prog;
    int ret = 1;

    if (!game_init("shaders")) return 1;
    if ((viewer = viewer_new(640, 480, ""))) {
        if ((prog = game_load_shader("standard.vert", "phong.frag", defines, sizeof(defines) / (2 * sizeof(*defines))))) {
            printf("================================================================================\n");
            printf("  %-44s %-16s %-16s\n", "name", "offset", "expected");
            printf("--------------------------------------------------------------------------------\n");
            ret = !test_camera(prog);
            printf("--------------------------------------------------------------------------------\n");
            ret = ret | !test_lights(prog);
            printf("================================================================================\n");
            glDeleteProgram(prog);
        }
        viewer_free(viewer);
    }
    game_free();
    return ret;
}
