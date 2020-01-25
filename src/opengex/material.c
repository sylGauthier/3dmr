#include <string.h>
#include <stdlib.h>
#include <game/render/texture.h>

#include "opengex_common.h"

enum PhongModes {
    PHONG_AMBIENT = 0,
    PHONG_DIFFUSE,
    PHONG_SPECULAR,
    PHONG_NB_PARAMS
};

int ogex_parse_color(struct ODDLStructure* cur, char** attrib, Vec3 col) {
    struct ODDLStructure* sub;
    struct ODDLProperty* prop;

    if (!(prop = oddl_get_property(cur, "attrib"))) {
        fprintf(stderr, "Error: Color: property \"attrib\" is required\n");
        return 0;
    }
    if (cur->nbStructures != 1) {
        fprintf(stderr, "Error: Color: invalid number of substructures\n");
        return 0;
    }
    sub = cur->structures[0];
    if (sub->type != TYPE_FLOAT32) {
        fprintf(stderr, "Error: Color: invalid substructure type, required FLOAT32\n");
        return 0;
    }
    if (sub->nbVec != 1 || (sub->vecSize != 3 && sub->vecSize != 4)) {
        fprintf(stderr, "Error: Color: invalid data in substructure, required 1 FLOAT[3/4]\n");
        return 0;
    }
    *attrib = prop->str;
    memcpy(col, sub->dataList, sizeof(Vec3));
    return 1;
}

int ogex_parse_texture(struct OgexContext* context, struct ODDLStructure* cur, char** attrib, GLuint* tex) {
    struct ODDLStructure* sub;
    struct ODDLProperty* prop;
    unsigned int i, texSet = 0;

    if (!(prop = oddl_get_property(cur, "attrib"))) {
        fprintf(stderr, "Error: Texture: missing property \"attrib\"\n");
        return 0;
    }
    *attrib = prop->str;
    for (i = 0; i < cur->nbStructures; i++) {
        sub = cur->structures[i];
        if (sub->type == TYPE_STRING) {
            char* name;
            if (!(sub->vecSize == 1 && sub->nbVec == 1)) {
                fprintf(stderr, "Error: Texture: invalid string data\n");
                return 0;
            }
            if (texSet) {
                fprintf(stderr, "Error: Texture: cannot have multiple textures\n");
                return 0;
            }
            name = ((char**)(sub->dataList))[0];
            if (name[0] == '/') {
                if (name[1] == '/') {
                    fprintf(stderr, "Error: Texture: //drive/path not supported\n");
                    return 0;
                }
                *tex = texture_load_from_png(name);
            } else if (!context->path) {
                *tex = texture_load_from_png(name);
            } else {
                char* path;
                size_t n = strlen(context->path), m = strlen(name), s;
                if (n > (((size_t)-1) - m) || (s = (n + m)) > ((size_t)-3) || !(path = malloc(s + 2))) {
                    fprintf(stderr, "Error: Texture: failed to allocated memory for texture path\n");
                    return 0;
                }
                memcpy(path, context->path, n);
                path[n++] = '/';
                memcpy(path + n, name, m);
                path[s + 1] = 0;
                *tex = texture_load_from_png(path);
                free(path);
            }
            if (!*tex) {
                fprintf(stderr, "Error: Texture: could not load texture file: %s\n", name);
                return 0;
            }
            texSet = 1;
        } else {
            switch (ogex_get_identifier(sub)) {
                case OGEX_TRANSFORM:
                case OGEX_TRANSLATION:
                case OGEX_ROTATION:
                case OGEX_SCALE:
                    fprintf(stderr, "Warning: Texture: transformations are not supported yet\n");
                    break;
                case OGEX_ANIMATION:
                    fprintf(stderr, "Warning: Texture: animations are not supported yet\n");
                    break;
                default:
                    break;
            }
        }
    }
    if (!texSet) {
        fprintf(stderr, "Error: Texture: needs one filename\n");
        return 0;
    }
    return 1;
}

static enum PhongModes get_phong_mode(const char* attrib) {
    if (!strcmp(attrib, "diffuse")) return PHONG_DIFFUSE;
    if (!strcmp(attrib, "specular")) return PHONG_SPECULAR;
    if (!strcmp(attrib, "ambient")) return PHONG_AMBIENT;
    return PHONG_NB_PARAMS;
}

int ogex_parse_material(struct OgexContext* context, struct ODDLStructure* cur) {
    unsigned int i;
    struct MatParamVec3* params[PHONG_NB_PARAMS];
    struct PhongMaterialParams* phongParams;

    if (!(phongParams = phong_material_params_new())) return 0;
    params[PHONG_AMBIENT] = &phongParams->ambient;
    params[PHONG_DIFFUSE] = &phongParams->diffuse;
    params[PHONG_SPECULAR] = &phongParams->specular;

    for (i = 0; i < cur->nbStructures; i++) {
        struct ODDLStructure* tmp = cur->structures[i];
        Vec3 color;
        float param;
        char* attrib;
        GLuint tex = 0;
        enum PhongModes mode;

        switch (ogex_get_identifier(tmp)) {
            case OGEX_COLOR:
                if (!(ogex_parse_color(tmp, &attrib, color))) return 0;
                switch ((mode = get_phong_mode(attrib))) {
                    case PHONG_DIFFUSE:
                    case PHONG_SPECULAR:
                    case PHONG_AMBIENT:
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
                break;
            case OGEX_PARAM:
                if (!ogex_parse_param(tmp, &attrib, &param)) return 0;
                if (!strcmp(attrib, "specular_power")) {
                    material_param_set_float_constant(&phongParams->shininess, param);
                } else {
                    fprintf(stderr, "Warning: Material: unsupported Param attribute: %s\n", attrib);
                }
                break;
            case OGEX_TEXTURE:
                if (!ogex_parse_texture(context, tmp, &attrib, &tex)) return 0;
                switch ((mode = get_phong_mode(attrib))) {
                    case PHONG_DIFFUSE:
                    case PHONG_SPECULAR:
                    case PHONG_AMBIENT:
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
                break;
            default:
                break;
        }
    }
    if (!ogex_add_shared_object(context, cur, phongParams, 1)) return 0;
    if (context->shared) {
        if (!import_add_shared_item(&context->shared->matParams, &context->shared->nbMatParams, phongParams)) {
            free(phongParams);
            return 0;
        }
    }
    return 1;
}
