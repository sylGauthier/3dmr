#include <stdlib.h>
#include <game/init.h>
#include <game/material/phong.h>
#include <game/render/shader.h>
#include <game/render/vertex_shader.h>

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

void phong_load(GLuint program, void* params) {
    const struct PhongMaterialParams* p = params;
    unsigned int texSlot = 0;
    material_param_send_vec3(program, &p->ambient, "ambient", &texSlot);
    material_param_send_vec3(program, &p->diffuse, "diffuse", &texSlot);
    material_param_send_vec3(program, &p->specular, "specular", &texSlot);
    material_param_send_float(program, &p->shininess, "shininess", &texSlot);
    if (p->normalMap) {
        material_param_send_texture(program, p->normalMap, "normalMap", &texSlot);
    }
    alpha_params_send(program, &p->alpha, &texSlot);
}

GLuint phong_shader_new(const struct PhongMaterialParams* params) {
    static const char* defines[2 * (5 + ALPHA_MAX_DEFINES)];
    unsigned int numDefines = 0;

    if (params->normalMap) {
        defines[2 * numDefines] = "NORMALMAP";
        defines[2 * numDefines++ + 1] = NULL;
    }
    if (params->ambient.mode == MAT_PARAM_TEXTURED) {
        defines[2 * numDefines] = "AMBIENT_TEXTURED";
        defines[2 * numDefines++ + 1] = NULL;
    }
    if (params->diffuse.mode == MAT_PARAM_TEXTURED) {
        defines[2 * numDefines] = "DIFFUSE_TEXTURED";
        defines[2 * numDefines++ + 1] = NULL;
    }
    if (params->specular.mode == MAT_PARAM_TEXTURED) {
        defines[2 * numDefines] = "SPECULAR_TEXTURED";
        defines[2 * numDefines++ + 1] = NULL;
    }
    if (params->shininess.mode == MAT_PARAM_TEXTURED) {
        defines[2 * numDefines] = "SHININESS_TEXTURED";
        defines[2 * numDefines++ + 1] = NULL;
    }
    alpha_set_defines(&params->alpha, defines, &numDefines);
    return shader_find_compile("phong.frag", GL_FRAGMENT_SHADER, &shaderRootPath, 1, defines, numDefines);
}

struct Material* phong_material_new(enum MeshFlags mflags, const struct PhongMaterialParams* params) {
    struct Material* m = NULL;
    GLuint shaders[2];
    shaders[0] = vertex_shader_standard(mflags);
    shaders[1] = phong_shader_new(params);
    if (shaders[0] && shaders[1]) m = material_new_from_shaders(shaders, 2, 0, phong_load, 0, params, GL_FILL);
    if (shaders[0]) glDeleteShader(shaders[0]);
    if (shaders[1]) glDeleteShader(shaders[1]);
    return m;
}

int material_is_phong(const struct Material* material) {
    return material->frag_load == phong_load;
}
