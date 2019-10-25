#include <stdlib.h>
#include <game/material/pbr.h>
#include <game/render/camera_buffer_object.h>
#include <game/render/lights_buffer_object.h>
#include <game/render/viewer.h>
#include "programs.h"

static void pbr_load(const struct Material* material) {
    const struct PBRMaterial* m = (const struct PBRMaterial*)material;
    unsigned int texSlot = 0;
    material_param_send_vec3(material->program,  &m->albedo, "albedo", &texSlot);
    material_param_send_float(material->program, &m->metalness, "metalness", &texSlot);
    material_param_send_float(material->program, &m->roughness, "roughness", &texSlot);
    if (m->normalMap) {
        material_param_send_texture(material->program, m->normalMap, "normalMap", &texSlot);
    }
    alpha_params_send(material->program, &m->alpha, &texSlot);
    light_load_ibl_uniforms(material->program, m->ibl, texSlot, texSlot + 1, texSlot + 2);
}

struct PBRMaterial* pbr_material_new(enum PBRMaterialFlags flags) {
    static const char* defines[2 * (6 + ALPHA_MAX_DEFINES)];
    unsigned int numDefines = 0;
    struct Viewer* currentViewer;
    struct PBRMaterial* pbrMat;
    GLuint prog;
    unsigned int progid, variant = flags & 15;

    defines[2 * numDefines] = "HAVE_NORMAL";
    defines[2 * numDefines++ + 1] = NULL;
    if (flags & PBR_NORMALMAP) {
        defines[2 * numDefines] = "HAVE_TANGENT";
        defines[2 * numDefines++ + 1] = NULL;
    }
    if (flags & PBR_ALBEDO_TEXTURED) {
        defines[2 * numDefines] = "ALBEDO_TEXTURED";
        defines[2 * numDefines++ + 1] = NULL;
    }
    if (flags & PBR_METALNESS_TEXTURED) {
        defines[2 * numDefines] = "METALNESS_TEXTURED";
        defines[2 * numDefines++ + 1] = NULL;
    }
    if (flags & PBR_ROUGHNESS_TEXTURED) {
        defines[2 * numDefines] = "ROUGHNESS_TEXTURED";
        defines[2 * numDefines++ + 1] = NULL;
    }
    if (variant) {
        defines[2 * numDefines] = "HAVE_TEXCOORD";
        defines[2 * numDefines++ + 1] = NULL;
    }
    alpha_set_defines((enum AlphaParamsFlags)flags, defines, &numDefines);
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
    if (!(pbrMat = malloc(sizeof(*pbrMat)))) {
        return NULL;
    }
    pbrMat->material.load = pbr_load;
    pbrMat->material.program = prog;
    pbrMat->material.polygonMode = GL_FILL;
    pbrMat->normalMap = 0;
    pbrMat->ibl = NULL;

    return pbrMat;
}

int material_is_pbr(const struct Material* material) {
    return material->load == pbr_load;
}
