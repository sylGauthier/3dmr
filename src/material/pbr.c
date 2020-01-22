#include <stdlib.h>
#include <game/init.h>
#include <game/material/pbr.h>
#include <game/render/shader.h>
#include <game/render/vertex_shader.h>

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

void pbr_load(GLuint program, void* params) {
    const struct PBRMaterialParams* p = params;
    unsigned int texSlot = 0;
    material_param_send_vec3(program,  &p->albedo, "albedo", &texSlot);
    material_param_send_float(program, &p->metalness, "metalness", &texSlot);
    material_param_send_float(program, &p->roughness, "roughness", &texSlot);
    if (p->normalMap) {
        material_param_send_texture(program, p->normalMap, "normalMap", &texSlot);
    }
    alpha_params_send(program, &p->alpha, &texSlot);
    light_load_ibl_uniforms(program, p->ibl, texSlot, texSlot + 1, texSlot + 2);
}

GLuint pbr_shader_new(const struct PBRMaterialParams* params) {
    static const char* defines[2 * (4 + ALPHA_MAX_DEFINES)];
    unsigned int numDefines = 0;

    if (params->normalMap) {
        defines[2 * numDefines] = "NORMALMAP";
        defines[2 * numDefines++ + 1] = NULL;
    }
    if (params->albedo.mode == MAT_PARAM_TEXTURED) {
        defines[2 * numDefines] = "ALBEDO_TEXTURED";
        defines[2 * numDefines++ + 1] = NULL;
    }
    if (params->metalness.mode == MAT_PARAM_TEXTURED) {
        defines[2 * numDefines] = "METALNESS_TEXTURED";
        defines[2 * numDefines++ + 1] = NULL;
    }
    if (params->roughness.mode == MAT_PARAM_TEXTURED) {
        defines[2 * numDefines] = "ROUGHNESS_TEXTURED";
        defines[2 * numDefines++ + 1] = NULL;
    }
    alpha_set_defines(&params->alpha, defines, &numDefines);
    return shader_find_compile("pbr.frag", GL_FRAGMENT_SHADER, &shaderRootPath, 1, defines, numDefines);
}

struct Material* pbr_material_new(enum MeshFlags mflags, const struct PBRMaterialParams* params) {
    struct Material* m = NULL;
    GLuint shaders[2];
    shaders[0] = vertex_shader_standard(mflags);
    shaders[1] = pbr_shader_new(params);
    if (shaders[0] && shaders[1]) m = material_new_from_shaders(shaders, 2, 0, pbr_load, 0, params, GL_FILL);
    if (shaders[0]) glDeleteShader(shaders[0]);
    if (shaders[1]) glDeleteShader(shaders[1]);
    return m;
}

int material_is_pbr(const struct Material* material) {
    return material->frag_load == pbr_load;
}
