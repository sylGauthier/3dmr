#include <string.h>
#include <stdlib.h>
#include "color.h"
#include "common.h"
#include "material.h"
#include "param.h"
#include "texture.h"

enum PhongModes {
    PHONG_AMBIENT = 0,
    PHONG_DIFFUSE,
    PHONG_SPECULAR,
    PHONG_NB_PARAMS
};

enum PhongFlags {
    PHONG_AMBIENT_SET = 1 << PHONG_AMBIENT,
    PHONG_DIFFUSE_SET = 1 << PHONG_DIFFUSE,
    PHONG_SPECULAR_SET = 1 << PHONG_SPECULAR
};

static enum PhongModes get_phong_mode(const char* attrib) {
    if (!strcmp(attrib, "diffuse")) return PHONG_DIFFUSE;
    if (!strcmp(attrib, "specular")) return PHONG_SPECULAR;
    if (!strcmp(attrib, "ambient")) return PHONG_AMBIENT;
    return PHONG_NB_PARAMS;
}

struct PhongMaterialParams* ogex_parse_material(const struct OgexContext* context, const struct ODDLStructure* cur) {
    struct MatParamVec3* params[PHONG_NB_PARAMS];
    struct PhongMaterialParams* phongParams;
    unsigned int i, phongFlags = 0;
    int ok = 1;

    if (!(phongParams = phong_material_params_new())) return NULL;
    params[PHONG_AMBIENT] = &phongParams->ambient;
    params[PHONG_DIFFUSE] = &phongParams->diffuse;
    params[PHONG_SPECULAR] = &phongParams->specular;

    for (i = 0; ok && i < cur->nbStructures; i++) {
        struct ODDLStructure* tmp = cur->structures[i];
        Vec3 color;
        float param;
        char* attrib;
        GLuint tex = 0;
        enum PhongModes mode;

        if (!tmp->identifier) {
            continue;
        } else if (!strcmp(tmp->identifier, "Color")) {
            if (!(ogex_parse_color(tmp, &attrib, color))) {
                ok = 0;
            } else switch ((mode = get_phong_mode(attrib))) {
                case PHONG_DIFFUSE:
                case PHONG_SPECULAR:
                case PHONG_AMBIENT:
                    phongFlags |= 1 << mode;
                    if (params[mode]->mode == MAT_PARAM_TEXTURED) break;
                    material_param_set_vec3_constant(params[mode], color);
                    break;
                default:
                    if (!strcmp(attrib, "opacity")) {
                        fprintf(stderr, "Warning: Material: opacity attribute not supported\n");
                    } else if (!strcmp(attrib, "transparency")) {
                        fprintf(stderr, "Warning: Material: transparency attribute not supported\n");
                    } else {
                        fprintf(stderr, "Warning: Material: attribute not supported for Color: %s\n", attrib);
                    }
                    break;
            }
        } else if (!strcmp(tmp->identifier, "Param")) {
            if (!ogex_parse_param(tmp, &attrib, &param)) {
                ok = 0;
            } else if (!strcmp(attrib, "specular_power")) {
                material_param_set_float_constant(&phongParams->shininess, param);
            } else {
                fprintf(stderr, "Warning: Material: unsupported Param attribute: %s\n", attrib);
            }
        } else if (!strcmp(tmp->identifier, "Texture")) {
            if (!ogex_parse_texture(context, tmp, &attrib, &tex)) {
                ok = 0;
            } else switch ((mode = get_phong_mode(attrib))) {
                case PHONG_DIFFUSE:
                case PHONG_SPECULAR:
                case PHONG_AMBIENT:
                    phongFlags |= 1 << mode;
                    if (params[mode]->mode == MAT_PARAM_TEXTURED) {
                        fprintf(stderr, "Warning: Material: multiple textures for same phong attribute not supported\n");
                        glDeleteTextures(1, &tex);
                    } else {
                        material_param_set_vec3_texture(params[mode], tex);
                    }
                    break;
                default:
                    fprintf(stderr, "Warning: Material: attribute not supported for Texture\n");
                    glDeleteTextures(1, &tex);
                    break;
            }
        }
    }
    if (!ok) {
        ogex_free_material(phongParams);
        return NULL;
    }
    /* If diffuse is set but not ambient, copy diffuse params to ambient params to make it behave more intuitively */
    if ((phongFlags & PHONG_DIFFUSE_SET) && !(phongFlags & PHONG_AMBIENT_SET)) {
        memcpy(params[PHONG_AMBIENT], params[PHONG_DIFFUSE], sizeof(*params[PHONG_AMBIENT]));
    }
    return phongParams;
}

void ogex_free_material(void* p) {
    struct PhongMaterialParams* phongParams = p;
    struct MatParamVec3* params[PHONG_NB_PARAMS];
    unsigned int i;

    params[PHONG_AMBIENT] = &phongParams->ambient;
    params[PHONG_DIFFUSE] = &phongParams->diffuse;
    params[PHONG_SPECULAR] = &phongParams->specular;
    for (i = 0; i < PHONG_NB_PARAMS; i++) {
        if (params[i]->mode == MAT_PARAM_TEXTURED) {
            glDeleteTextures(1, &params[i]->value.texture);
        }
    }
    free(phongParams);
}
