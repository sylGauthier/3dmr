#include <stdlib.h>
#include <game/material/phong.h>
#include <game/render/camera_buffer_object.h>
#include <game/render/lights_buffer_object.h>
#include <game/render/viewer.h>
#include "programs.h"

void phong_material_params_init(struct PhongMaterialParams* p) {
    material_param_set_vec3_elems(&p->ambient, 0, 0, 0);
    material_param_set_vec3_elems(&p->diffuse, 1, 1, 1);
    material_param_set_vec3_elems(&p->specular, 0, 0, 0);
    material_param_set_float_constant(&p->shininess, 1);
    alpha_params_init(&p->alpha);
    p->normalMap = 0;
}

struct PhongMaterialParams* phong_material_params_new(void) {
    struct PhongMaterialParams* p;
    if ((p = malloc(sizeof(*p)))) {
        phong_material_params_init(p);
    }
    return p;
}

static void phong_load(const struct Material* material) {
    const struct PhongMaterialParams* p = material->params;
    unsigned int texSlot = 0;
    material_param_send_vec3(material->program, &p->ambient, "ambient", &texSlot);
    material_param_send_vec3(material->program, &p->diffuse, "diffuse", &texSlot);
    material_param_send_vec3(material->program, &p->specular, "specular", &texSlot);
    material_param_send_float(material->program, &p->shininess, "shininess", &texSlot);
    if (p->normalMap) {
        material_param_send_texture(material->program, p->normalMap, "normalMap", &texSlot);
    }
    alpha_params_send(material->program, &p->alpha, &texSlot);
}

struct Material* phong_material_new(struct PhongMaterialParams* params) {
    static const char* defines[2 * (7 + ALPHA_MAX_DEFINES)];
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
    if (params->ambient.mode == MAT_PARAM_TEXTURED) {
        defines[2 * numDefines] = "AMBIENT_TEXTURED";
        defines[2 * numDefines++ + 1] = NULL;
        variant |= 2;
    }
    if (params->diffuse.mode == MAT_PARAM_TEXTURED) {
        defines[2 * numDefines] = "DIFFUSE_TEXTURED";
        defines[2 * numDefines++ + 1] = NULL;
        variant |= 4;
    }
    if (params->specular.mode == MAT_PARAM_TEXTURED) {
        defines[2 * numDefines] = "SPECULAR_TEXTURED";
        defines[2 * numDefines++ + 1] = NULL;
        variant |= 8;
    }
    if (params->shininess.mode == MAT_PARAM_TEXTURED) {
        defines[2 * numDefines] = "SHININESS_TEXTURED";
        defines[2 * numDefines++ + 1] = NULL;
        variant |= 16;
    }
    if (variant) {
        defines[2 * numDefines] = "HAVE_TEXCOORD";
        defines[2 * numDefines++ + 1] = NULL;
    }
    alpha_set_defines(&params->alpha, defines, &numDefines);

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

    return material_new(phong_load, params, prog, GL_FILL);
}

int material_is_phong(const struct Material* material) {
    return material->load == phong_load;
}
