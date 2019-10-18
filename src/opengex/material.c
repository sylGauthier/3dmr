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

static void init_phong_params(struct MatParamVec3 params[]) {
    params[PHONG_AMBIENT].mode = MAT_PARAM_CONSTANT;
    params[PHONG_AMBIENT].value.constant[0] = 0.0;
    params[PHONG_AMBIENT].value.constant[1] = 0.0;
    params[PHONG_AMBIENT].value.constant[2] = 0.0;

    params[PHONG_DIFFUSE].mode = MAT_PARAM_CONSTANT;
    params[PHONG_DIFFUSE].value.constant[0] = 1.0;
    params[PHONG_DIFFUSE].value.constant[1] = 1.0;
    params[PHONG_DIFFUSE].value.constant[2] = 1.0;

    params[PHONG_SPECULAR].mode = MAT_PARAM_CONSTANT;
    params[PHONG_SPECULAR].value.constant[0] = 0.0;
    params[PHONG_SPECULAR].value.constant[1] = 0.0;
    params[PHONG_SPECULAR].value.constant[2] = 0.0;
}

static enum PhongModes get_phong_mode(const char* attrib) {
    if (!strcmp(attrib, "diffuse")) return PHONG_DIFFUSE;
    if (!strcmp(attrib, "specular")) return PHONG_SPECULAR;
    if (!strcmp(attrib, "ambient")) return PHONG_AMBIENT;
    return PHONG_NB_PARAMS;
}

int ogex_parse_material(struct OgexContext* context, struct ODDLStructure* cur) {
    unsigned int i;
    struct MatParamVec3 params[PHONG_NB_PARAMS];
    struct PhongMaterial* newMat;
    float spec = 1.0;
    char flags = 0;

    if (ogex_get_shared_object(context, cur)) return 1;

    init_phong_params(params);

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
                        if ((flags & (1 << mode)) && params[mode].mode == MAT_PARAM_TEXTURED) {
                            break;
                        }
                        material_param_set_vec3_constant(params + mode, color);
                        flags |= 1 << mode;
                        break;
                    default:
                        if (!strcmp(attrib, "specular")) {
                            params[PHONG_SPECULAR].mode = MAT_PARAM_CONSTANT;
                            memcpy(params[PHONG_SPECULAR].value.constant, color, sizeof(Vec3));
                            flags |= 1 << PHONG_SPECULAR;
                        } else if (!strcmp(attrib, "ambient")) {
                            params[PHONG_AMBIENT].mode = MAT_PARAM_CONSTANT;
                            memcpy(params[PHONG_AMBIENT].value.constant, color, sizeof(Vec3));
                            flags |= 1 << PHONG_AMBIENT;
                        } else if (!strcmp(attrib, "opacity")) {
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
                    spec = param;
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
                        if ((flags & (1 << mode)) && params[mode].mode == MAT_PARAM_TEXTURED) {
                            fprintf(stderr, "Warning: Material: multiple textures for same phong attribute not supported\n");
                            glDeleteTextures(1, &tex);
                            return 0;
                        }
                        material_param_set_vec3_texture(params + mode, tex);
                        flags |= 1 << mode;
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
    newMat = phong_material_new(params[PHONG_AMBIENT].mode, params[PHONG_DIFFUSE].mode, params[PHONG_SPECULAR].mode, MAT_PARAM_CONSTANT, 0);
    newMat->ambient = params[PHONG_AMBIENT];
    newMat->diffuse = params[PHONG_DIFFUSE];
    newMat->specular = params[PHONG_SPECULAR];
    material_param_set_float_constant(&newMat->shininess, spec);
    if (!ogex_add_shared_object(context, cur, newMat, 1)) return 0;
    if (context->shared) {
        if (!import_add_shared_item(&context->shared->mats, &context->shared->nbMat, newMat)) {
            return 0;
        }
    }
    return 1;
}
