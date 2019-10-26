#include <stdlib.h>
#include <game/material/solid.h>
#include <game/render/camera_buffer_object.h>
#include <game/render/viewer.h>
#include "programs.h"

void solid_material_params_init(struct SolidMaterialParams* p) {
    material_param_set_vec3_elems(&p->color, 1, 1, 1);
    alpha_params_init(&p->alpha);
    p->options = 0;
}

struct SolidMaterialParams* solid_material_params_new(void) {
    struct SolidMaterialParams* p;
    if ((p = malloc(sizeof(*p)))) {
        solid_material_params_init(p);
    }
    return p;
}

static void solid_load(const struct Material* material) {
    const struct SolidMaterialParams* p = material->params;
    unsigned int texSlot = 0;
    material_param_send_vec3(material->program, &p->color, "solidColor", &texSlot);
    alpha_params_send(material->program, &p->alpha, &texSlot);
}

struct Material* solid_material_new(struct SolidMaterialParams* params) {
    static const char* defines[2 * (3 + ALPHA_MAX_DEFINES)];
    unsigned int numDefines = 0;
    struct Viewer* currentViewer;
    GLuint prog;
    unsigned int progid, variant = 0;

    if (params->color.mode == MAT_PARAM_TEXTURED) {
        defines[2 * numDefines] = "HAVE_TEXCOORD";
        defines[2 * numDefines++ + 1] = NULL;
        defines[2 * numDefines] = "SOLID_TEXTURED";
        defines[2 * numDefines++ + 1] = NULL;
        variant |= 1;
    }
    if (params->options & SOLID_OVERLAY) {
        defines[2 * numDefines] = "OVERLAY";
        defines[2 * numDefines++ + 1] = NULL;
        variant |= 2;
    }
    alpha_set_defines(&params->alpha, defines, &numDefines);

    if ((progid = viewer_get_program_id(GAME_UID_SOLID, variant)) == ((unsigned int)-1)
     ||!(currentViewer = viewer_get_current())) {
        return NULL;
    }
    if (!(prog = viewer_get_program(currentViewer, progid))) {
        if (!(prog = game_load_shader("standard.vert", "solid.frag", defines, numDefines))) {
            return NULL;
        }
        glUniformBlockBinding(prog, glGetUniformBlockIndex(prog, "Camera"), CAMERA_UBO_BINDING);
        if (!viewer_set_program(currentViewer, progid, prog)) {
            glDeleteProgram(prog);
            return NULL;
        }
    }

    return material_new(solid_load, params, prog, GL_FILL);
}

int material_is_solid(const struct Material* material) {
    return material->load == solid_load;
}
