#include <stdlib.h>
#include <game/material/phong.h>
#include <game/render/camera_buffer_object.h>
#include <game/render/lights_buffer_object.h>
#include <game/render/viewer.h>
#include "programs.h"

static void phong_load(const struct Material* material) {
    const struct PhongMaterial* m = (const struct PhongMaterial*)material;
    unsigned int texSlot = 0;
    material_param_send_vec3(material->program, &m->ambient, "ambient", &texSlot);
    material_param_send_vec3(material->program, &m->diffuse, "diffuse", &texSlot);
    material_param_send_vec3(material->program, &m->specular, "specular", &texSlot);
    material_param_send_float(material->program, &m->shininess, "shininess", &texSlot);
    if (m->normalMap) {
        material_param_send_texture(material->program, m->normalMap, "normalMap", &texSlot);
    }
    alpha_params_send(material->program, &m->alpha, &texSlot);
}

struct PhongMaterial* phong_material_new(enum PhongMaterialFlags flags) {
    static const char* defines[2 * (7 + ALPHA_MAX_DEFINES)];
    unsigned int numDefines = 0;
    struct Viewer* currentViewer;
    struct PhongMaterial* phong;
    GLuint prog;
    unsigned int progid, variant = flags & 31;

    defines[2 * numDefines] = "HAVE_NORMAL";
    defines[2 * numDefines++ + 1] = NULL;
    if (flags & PHONG_NORMALMAP) {
        defines[2 * numDefines] = "HAVE_TANGENT";
        defines[2 * numDefines++ + 1] = NULL;
    }
    if (flags & PHONG_AMBIENT_TEXTURED) {
        defines[2 * numDefines] = "AMBIENT_TEXTURED";
        defines[2 * numDefines++ + 1] = NULL;
    }
    if (flags & PHONG_DIFFUSE_TEXTURED) {
        defines[2 * numDefines] = "DIFFUSE_TEXTURED";
        defines[2 * numDefines++ + 1] = NULL;
    }
    if (flags & PHONG_SPECULAR_TEXTURED) {
        defines[2 * numDefines] = "SPECULAR_TEXTURED";
        defines[2 * numDefines++ + 1] = NULL;
    }
    if (flags & PHONG_SHININESS_TEXTURED) {
        defines[2 * numDefines] = "SHININESS_TEXTURED";
        defines[2 * numDefines++ + 1] = NULL;
    }
    if (variant) {
        defines[2 * numDefines] = "HAVE_TEXCOORD";
        defines[2 * numDefines++ + 1] = NULL;
    }
    alpha_set_defines((enum AlphaParamsFlags)flags, defines, &numDefines);
    if ((progid = viewer_get_program_id(GAME_UID_PHONG, variant)) == ((unsigned int)-1)
     || !(currentViewer = viewer_get_current())) {
        return NULL;
    }
    if (!(prog = viewer_get_program(currentViewer, progid))) {
        if (!(prog = game_load_shader("standard.vert", "phong.frag", defines, numDefines))) {
            return NULL;
        }
        glUniformBlockBinding(prog, glGetUniformBlockIndex(prog, "Camera"), CAMERA_UBO_BINDING);
        glUniformBlockBinding(prog, glGetUniformBlockIndex(prog, "Lights"), LIGHTS_UBO_BINDING);
        if (!viewer_set_program(currentViewer, progid, prog)) {
            glDeleteProgram(prog);
            return NULL;
        }
    }
    if (!(phong = malloc(sizeof(*phong)))) {
        return NULL;
    }
    phong->material.load = phong_load;
    phong->material.program = prog;
    phong->material.polygonMode = GL_FILL;
    phong->normalMap = 0;

    return phong;
}

int material_is_phong(const struct Material* material) {
    return material->load == phong_load;
}
