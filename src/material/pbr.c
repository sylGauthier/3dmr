#include <stdlib.h>
#include <game/material/pbr.h>
#include <game/render/camera_buffer_object.h>
#include <game/render/lights_buffer_object.h>
#include <game/render/viewer.h>
#include "programs.h"

void pbr_material_params_init(struct PBRMaterialParams* p) {
    material_param_set_vec3_elems(&p->albedo, 1, 1, 1);
    material_param_set_float_constant(&p->metalness, 0);
    material_param_set_float_constant(&p->roughness, 0.5);
    alpha_params_init(&p->alpha);
    p->normalMap = 0;
    p->ibl = NULL;
}

struct PBRMaterialParams* pbr_material_params_new(void) {
    struct PBRMaterialParams* p;
    if ((p = malloc(sizeof(*p)))) {
        pbr_material_params_init(p);
    }
    return p;
}

static void pbr_load(const struct Material* material) {
    const struct PBRMaterialParams* p = material->params;
    unsigned int texSlot = 0;
    material_param_send_vec3(material->program,  &p->albedo, "albedo", &texSlot);
    material_param_send_float(material->program, &p->metalness, "metalness", &texSlot);
    material_param_send_float(material->program, &p->roughness, "roughness", &texSlot);
    if (p->normalMap) {
        material_param_send_texture(material->program, p->normalMap, "normalMap", &texSlot);
    }
    alpha_params_send(material->program, &p->alpha, &texSlot);
    light_load_ibl_uniforms(material->program, p->ibl, texSlot, texSlot + 1, texSlot + 2);
}

struct Material* pbr_material_new(struct PBRMaterialParams* params) {
    static const char* defines[2 * (6 + ALPHA_MAX_DEFINES)];
    unsigned int numDefines = 0;
    struct Viewer* currentViewer;
    GLuint prog;
    unsigned int progid, variant = 0;

    defines[2 * numDefines] = "HAVE_NORMAL";
    defines[2 * numDefines++ + 1] = NULL;
    if (params->normalMap) {
        defines[2 * numDefines] = "HAVE_TANGENT";
        defines[2 * numDefines++ + 1] = NULL;
        variant |= 1;
    }
    if (params->albedo.mode == MAT_PARAM_TEXTURED) {
        defines[2 * numDefines] = "ALBEDO_TEXTURED";
        defines[2 * numDefines++ + 1] = NULL;
        variant |= 2;
    }
    if (params->metalness.mode == MAT_PARAM_TEXTURED) {
        defines[2 * numDefines] = "METALNESS_TEXTURED";
        defines[2 * numDefines++ + 1] = NULL;
        variant |= 4;
    }
    if (params->roughness.mode == MAT_PARAM_TEXTURED) {
        defines[2 * numDefines] = "ROUGHNESS_TEXTURED";
        defines[2 * numDefines++ + 1] = NULL;
        variant |= 8;
    }
    if (variant) {
        defines[2 * numDefines] = "HAVE_TEXCOORD";
        defines[2 * numDefines++ + 1] = NULL;
    }
    alpha_set_defines(&params->alpha, defines, &numDefines);

    if ((progid = viewer_get_program_id(GAME_UID_PBR, variant)) == ((unsigned int)-1)
     || !(currentViewer = viewer_get_current())) {
        return NULL;
    }
    if (!(prog = viewer_get_program(currentViewer, progid))) {
        if (!(prog = game_load_shader("standard.vert", "pbr.frag", defines, numDefines))) {
            return NULL;
        }
        glUniformBlockBinding(prog, glGetUniformBlockIndex(prog, "Camera"), CAMERA_UBO_BINDING);
        glUniformBlockBinding(prog, glGetUniformBlockIndex(prog, "Lights"), LIGHTS_UBO_BINDING);
        if (!viewer_set_program(currentViewer, progid, prog)) {
            glDeleteProgram(prog);
            return NULL;
        }
    }

    return material_new(pbr_load, params, prog, GL_FILL);
}

int material_is_pbr(const struct Material* material) {
    return material->load == pbr_load;
}
