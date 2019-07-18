#include <stdlib.h>
#include <game/material/pbr.h>
#include <game/render/camera_buffer_object.h>
#include <game/render/lights_buffer_object.h>
#include <game/render/viewer.h>
#include "programs.h"

static unsigned int progid[16] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};

static void pbr_load(const struct Material* material) {
    unsigned int texSlot = 0;
    material_param_send_vec3(material->program, &((const struct PBRMaterial*)material)->albedo, "albedo", &texSlot);
    material_param_send_float(material->program, &((const struct PBRMaterial*)material)->metalness, "metalness", &texSlot);
    material_param_send_float(material->program, &((const struct PBRMaterial*)material)->roughness, "roughness", &texSlot);
    if (((const struct PBRMaterial*)material)->normalMap) {
        material_param_send_texture(material->program, ((const struct PBRMaterial*)material)->normalMap, "normalMap", &texSlot);
    }
    light_load_ibl_uniforms(material->program, ((const struct PBRMaterial*)material)->ibl, texSlot, texSlot + 1, texSlot + 2);
}

struct PBRMaterial* pbr_material_new(enum MatParamMode modeAlbedo, enum MatParamMode modeMetalness, enum MatParamMode modeRoughness, int hasNormalMap) {
    static const char* defines[12];
    unsigned int numDefines = 0;
    struct Viewer* currentViewer;
    struct PBRMaterial* pbrMat;
    GLuint prog;
    unsigned int variant = 0;

    defines[2 * numDefines] = "HAVE_NORMAL";
    defines[2 * numDefines++ + 1] = NULL;
    if (hasNormalMap) {
        defines[2 * numDefines] = "HAVE_TANGENT";
        defines[2 * numDefines++ + 1] = NULL;
        variant |= 1 << 0;
    }
    if (modeAlbedo == MAT_PARAM_TEXTURED) {
        defines[2 * numDefines] = "ALBEDO_TEXTURED";
        defines[2 * numDefines++ + 1] = NULL;
        variant |= 1 << 1;
    }
    if (modeMetalness == MAT_PARAM_TEXTURED) {
        defines[2 * numDefines] = "METALNESS_TEXTURED";
        defines[2 * numDefines++ + 1] = NULL;
        variant |= 1 << 2;
    }
    if (modeRoughness == MAT_PARAM_TEXTURED) {
        defines[2 * numDefines] = "ROUGHNESS_TEXTURED";
        defines[2 * numDefines++ + 1] = NULL;
        variant |= 1 << 3;
    }
    if (numDefines > 1) {
        defines[2 * numDefines] = "HAVE_TEXCOORD";
        defines[2 * numDefines++ + 1] = NULL;
    }
    if (progid[variant] == ((unsigned int)-1)) {
        if ((progid[variant] = viewer_register_program_id()) == ((unsigned int)-1)) {
            return NULL;
        }
    }
    if (!(currentViewer = viewer_get_current())) {
        return NULL;
    }
    if (!(prog = viewer_get_program(currentViewer, progid[variant]))) {
        if (!(prog = game_load_shader("standard.vert", "pbr.frag", defines, numDefines))) {
            return NULL;
        }
        glUniformBlockBinding(prog, glGetUniformBlockIndex(prog, "Camera"), CAMERA_UBO_BINDING);
        glUniformBlockBinding(prog, glGetUniformBlockIndex(prog, "Lights"), LIGHTS_UBO_BINDING);
        if (!viewer_set_program(currentViewer, progid[variant], prog)) {
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
    pbrMat->albedo.mode = modeAlbedo;
    pbrMat->metalness.mode = modeMetalness;
    pbrMat->roughness.mode = modeRoughness;
    pbrMat->normalMap = 0;
    pbrMat->ibl = NULL;

    return pbrMat;
}
