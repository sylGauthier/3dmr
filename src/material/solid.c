#include <stdlib.h>
#include <game/init.h>
#include <game/material/solid.h>
#include <game/render/shader.h>

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

void solid_load(GLuint program, void* params) {
    const struct SolidMaterialParams* p = params;
    unsigned int texSlot = 0;
    material_param_send_vec3(program, &p->color, "solidColor", &texSlot);
    alpha_params_send(program, &p->alpha, &texSlot);
}

GLuint solid_shader_new(const struct SolidMaterialParams* params) {
    static const char* defines[2 * (2 + ALPHA_MAX_DEFINES)];
    unsigned int numDefines = 0;

    if (params->color.mode == MAT_PARAM_TEXTURED) {
        defines[2 * numDefines] = "SOLID_TEXTURED";
        defines[2 * numDefines++ + 1] = NULL;
    }
    if (params->options & SOLID_OVERLAY) {
        defines[2 * numDefines] = "OVERLAY";
        defines[2 * numDefines++ + 1] = NULL;
    }
    alpha_set_defines(&params->alpha, defines, &numDefines);
    return shader_find_compile("solid.frag", GL_FRAGMENT_SHADER, &shaderRootPath, 1, defines, numDefines);
}

int material_is_solid(const struct Material* material) {
    return material->load == solid_load;
}
