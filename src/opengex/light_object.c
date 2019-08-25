#include <string.h>
#include <stdlib.h>

#include "opengex_common.h"

struct Atten {
    enum AttenType {
        ATTEN_LINEAR,
        ATTEN_SMOOTH,
        ATTEN_INVERSE,
        ATTEN_INVERSE_SQUARE
    } type;
    enum AttenKind {
        ATTEN_K_DIST,
        ATTEN_K_ANGLE,
        ATTEN_K_COS
    } kind;
    float begin, end;
    float scale;
    float offset;
    float constant, linear, quadratic;
    float power;
};

static int ogex_parse_atten(struct ODDLStructure* cur, struct Atten* atten) {
    struct ODDLProperty* prop;
    unsigned int i;

    atten->type = ATTEN_LINEAR;
    atten->kind = ATTEN_K_DIST;
    atten->begin = 0.0; atten->end = 1.0;
    atten->scale = 1.0;
    atten->offset = 0.0;
    atten->constant = 0.0;
    atten->linear = 0.0;
    atten->quadratic = 1.0;
    atten->power = 1.0;

    if ((prop = oddl_get_property(cur, "curve"))) {
        if (!strcmp(prop->str, "linear")) {
            atten->type = ATTEN_LINEAR;
        } else if (!strcmp(prop->str, "smooth")) {
            atten->type = ATTEN_SMOOTH;
        } else if (!strcmp(prop->str, "inverse")) {
            atten->type = ATTEN_INVERSE;
        } else if (!strcmp(prop->str, "inverse_square")) {
            atten->type = ATTEN_INVERSE_SQUARE;
        } else {
            fprintf(stderr, "Error: Atten: unsupported curve property: prop->str\n");
            return 0;
        }
    }

    for (i = 0; i < cur->nbStructures; i++) {
        struct ODDLStructure* tmp = cur->structures[i];
        char* attrib;
        float value;

        switch (ogex_get_identifier(tmp)) {
            case OGEX_PARAM:
                if (!(ogex_parse_param(tmp, &attrib, &value))) {
                    return 0;
                }
                if (!(strcmp(attrib, "begin"))) {
                    atten->begin = value;
                } else if (!(strcmp(attrib, "end"))) {
                    atten->end = value;
                } else if (!(strcmp(attrib, "scale"))) {
                    atten->scale = value;
                } else if (!(strcmp(attrib, "offset"))) {
                    atten->offset = value;
                } else if (!(strcmp(attrib, "constant"))) {
                    atten->constant = value;
                } else if (!(strcmp(attrib, "linear"))) {
                    atten->linear = value;
                } else if (!(strcmp(attrib, "quadratic"))) {
                    atten->quadratic = value;
                } else if (!(strcmp(attrib, "power"))) {
                    atten->power = value;
                } else {
                    fprintf(stderr, "Warning: Atten: unknown param attribute: %s\n", attrib);
                }
                break;
            default:
                break;
        }
    }
    return 1;
}

int ogex_parse_light_object(struct OgexContext* context, struct ODDLStructure* cur) {
    struct ODDLProperty* prop;
    enum OgexLightType type;
    Vec3 color = {1.0, 1.0, 1.0};
    float intensity = 1.0;
    struct Atten atten = {ATTEN_INVERSE_SQUARE, ATTEN_K_DIST, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    unsigned int i;

    if (ogex_get_shared_object(context, cur)) return 1;
    if (!(prop = oddl_get_property(cur, "type"))) {
        fprintf(stderr, "Error: LightObject: needs a \"type\" property\n");
        return 0;
    }
    if (!strcmp(prop->str, "infinite")) {
        type = OGEX_LIGHT_INFINITE;
    } else if (!strcmp(prop->str, "point")) {
        type = OGEX_LIGHT_POINT;
    } else if (!strcmp(prop->str, "spot")) {
        type = OGEX_LIGHT_SPOT;
    } else {
        fprintf(stderr, "Error: LightObject: unknown light type: %s\n", prop->str);
        return 0;
    }

    for (i = 0; i < cur->nbStructures; i++) {
        struct ODDLStructure* tmp = cur->structures[i];
        float param;
        char* attrib;

        switch (ogex_get_identifier(tmp)) {
            case OGEX_COLOR:
                if (!(ogex_parse_color(tmp, &attrib, color))) return 0;
                if (strcmp(attrib, "light")) {
                    fprintf(stderr, "Warning: LightObject: unknown color attribute: %s\n", attrib);
                    color[0] = 1; color[1] = 1; color[2] = 1;
                }
                break;
            case OGEX_PARAM:
                if (!(ogex_parse_param(tmp, &attrib, &param))) return 0;
                if (strcmp(attrib, "intensity")) {
                    fprintf(stderr, "Warning: LightObject: unknown param attribute: %s\n", attrib);
                } else {
                    intensity = param;
                }
                break;
            case OGEX_TEXTURE:
                fprintf(stderr, "Warning: LightObject: light textures are not supported yet (but will be Soon(TM))\n");
                break;
            case OGEX_ATTEN:
                if (!(ogex_parse_atten(tmp, &atten))) return 0;
                break;
            default:
                break;
        }
    }
    switch (type) {
        case OGEX_LIGHT_INFINITE:
            {
                struct DirectionalLight* newLight;
                if (!(newLight = malloc(sizeof(*newLight)))) {
                    fprintf(stderr, "Error: LightObject: cannot allocate memory for new light\n");
                    return 0;
                }
                memcpy(newLight->color, color, sizeof(Vec3));
                scale3v(newLight->color, intensity);
                if (!(ogex_add_shared_object(context, cur, newLight, 1))) {
                    free(newLight);
                    return 0;
                }
                if (context->shared) {
                    if (!import_add_shared_item(&context->shared->dlights, &context->shared->nbDLights, newLight)) {
                        free(newLight);
                        return 0;
                    }
                }
            }
            break;
        case OGEX_LIGHT_POINT:
            {
                struct PointLight* newLight;
                if (!(newLight = malloc(sizeof(*newLight)))) {
                    fprintf(stderr, "Error: LightObject: cannot allocate memory for new light\n");
                    return 0;
                }
                switch (atten.type) {
                    case ATTEN_SMOOTH:
                    case ATTEN_LINEAR:
                    case ATTEN_INVERSE:
                        fprintf(stderr, "Warning: LightObject: unsupported attenuation type (only inverse square is supported)\n");
                    case ATTEN_INVERSE_SQUARE:
                        break;
                    default:
                        fprintf(stderr, "Error: LightObject: unknown attenuation type\n");
                        free(newLight);
                        return 0;
                }
                memcpy(newLight->color, color, sizeof(Vec3));
                scale3v(newLight->color, intensity);
                newLight->radius = atten.scale;
                if (!(ogex_add_shared_object(context, cur, newLight, 1))) {
                    free(newLight);
                    return 0;
                }
                if (context->shared) {
                    if (!import_add_shared_item(&context->shared->plights, &context->shared->nbPLights, newLight)) {
                        free(newLight);
                        return 0;
                    }
                }
            }
            break;
        case OGEX_LIGHT_SPOT:
            fprintf(stderr, "Warning: LightObject: unsupported light object: spot\n");
            break;
        default:
            fprintf(stderr, "Error: LightObject: unknown light object\n");
            return 0;
    }
    return 1;
}
