#include <stdlib.h>
#include <game/material/phong.h>
#include <game/render/camera_buffer_object.h>
#include <game/render/lights_buffer_object.h>
#include <game/render/viewer.h>
#include "programs.h"

static unsigned int progid[32] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};

static void phong_load(const struct Material* material) {
    unsigned int texSlot = 0;
    material_param_send_vec3(material->program, &((const struct PhongMaterial*)material)->ambient, "ambient", &texSlot);
    material_param_send_vec3(material->program, &((const struct PhongMaterial*)material)->diffuse, "diffuse", &texSlot);
    material_param_send_vec3(material->program, &((const struct PhongMaterial*)material)->specular, "specular", &texSlot);
    material_param_send_float(material->program, &((const struct PhongMaterial*)material)->shininess, "shininess", &texSlot);
    if (((const struct PhongMaterial*)material)->normalMap) {
        material_param_send_texture(material->program, ((const struct PhongMaterial*)material)->normalMap, "normalMap", &texSlot);
    }
}

struct PhongMaterial* phong_material_new(enum MatParamMode modeAmbient, enum MatParamMode modeDiffuse, enum MatParamMode modeSpecular, enum MatParamMode modeShininess, int hasNormalMap) {
    static const char* defines[14];
    unsigned int numDefines = 0;
    struct Viewer* currentViewer;
    struct PhongMaterial* phong;
    GLuint prog;
    unsigned int variant = 0;

    defines[2 * numDefines] = "HAVE_NORMAL";
    defines[2 * numDefines++ + 1] = NULL;
    if (hasNormalMap) {
        defines[2 * numDefines] = "HAVE_TANGENT";
        defines[2 * numDefines++ + 1] = NULL;
        variant |= 1 << 0;
    }
    if (modeAmbient == MAT_PARAM_TEXTURED) {
        defines[2 * numDefines] = "AMBIENT_TEXTURED";
        defines[2 * numDefines++ + 1] = NULL;
        variant |= 1 << 1;
    }
    if (modeDiffuse == MAT_PARAM_TEXTURED) {
        defines[2 * numDefines] = "DIFFUSE_TEXTURED";
        defines[2 * numDefines++ + 1] = NULL;
        variant |= 1 << 2;
    }
    if (modeSpecular == MAT_PARAM_TEXTURED) {
        defines[2 * numDefines] = "SPECULAR_TEXTURED";
        defines[2 * numDefines++ + 1] = NULL;
        variant |= 1 << 3;
    }
    if (modeShininess == MAT_PARAM_TEXTURED) {
        defines[2 * numDefines] = "SHININESS_TEXTURED";
        defines[2 * numDefines++ + 1] = NULL;
        variant |= 1 << 4;
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
        if (!(prog = game_load_shader("standard.vert", "phong.frag", defines, numDefines))) {
            return NULL;
        }
        glUniformBlockBinding(prog, glGetUniformBlockIndex(prog, "Camera"), CAMERA_UBO_BINDING);
        glUniformBlockBinding(prog, glGetUniformBlockIndex(prog, "Lights"), LIGHTS_UBO_BINDING);
        if (!viewer_set_program(currentViewer, progid[variant], prog)) {
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
    phong->ambient.mode = modeAmbient;
    phong->diffuse.mode = modeDiffuse;
    phong->specular.mode = modeSpecular;
    phong->shininess.mode = modeShininess;
    phong->normalMap = 0;

    return phong;
}
