#include <stdlib.h>
#include <game/material/solid.h>
#include <game/render/camera_buffer_object.h>
#include <game/render/viewer.h>
#include "programs.h"

static unsigned int progid[2] = {-1, -1};

static void solid_load(const struct Material* material) {
    unsigned int texSlot = 0;
    material_param_send_vec3(material->program, &((const struct SolidMaterial*)material)->color, "solidColor", &texSlot);
}
#include <stdio.h>
struct SolidMaterial* solid_material_new(enum SolidMaterialFlags flags) {
    static const char* defines[] = {"HAVE_TEXCOORD", NULL};
    struct Viewer* currentViewer;
    struct SolidMaterial* solid;
    GLuint prog;
    unsigned int variant = (flags & SOLID_TEXTURED);

    if (progid[variant] == ((unsigned int)-1)) {
        if ((progid[variant] = viewer_register_program_id()) == ((unsigned int)-1)) {
            return NULL;
        }
    }
    if (!(currentViewer = viewer_get_current())) {
        return NULL;
    }
    if (!(prog = viewer_get_program(currentViewer, progid[variant]))) {
        if (!(prog = game_load_shader("standard.vert", "solid.frag", defines, variant))) {
            return NULL;
        }
        glUniformBlockBinding(prog, glGetUniformBlockIndex(prog, "Camera"), CAMERA_UBO_BINDING);
        if (!viewer_set_program(currentViewer, progid[variant], prog)) {
            glDeleteProgram(prog);
            return NULL;
        }
    }
    if (!(solid = malloc(sizeof(*solid)))) {
        return NULL;
    }
    solid->material.load = solid_load;
    solid->material.program = prog;
    solid->material.polygonMode = GL_FILL;

    return solid;
}

int material_is_solid(const struct Material* material) {
    return material->load == solid_load;
}
