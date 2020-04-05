#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "color.h"
#include "context.h"
#include "light_object.h"
#include "param.h"

static int ogex_parse_atten(const struct ODDLStructure* cur, struct OgexAtten* atten) {
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

    if ((prop = oddl_get_property(cur, "kind")) && prop->str) {
        if (!strcmp(prop->str, "distance")) {
            atten->kind = ATTEN_K_DIST;
        } else if (!strcmp(prop->str, "angle")) {
            atten->kind = ATTEN_K_ANGLE;
        } else if (!strcmp(prop->str, "cos_angle")) {
            atten->kind = ATTEN_K_COS;
        }
    }

    if ((prop = oddl_get_property(cur, "curve")) && prop->str) {
        if (strcmp(prop->str, "linear")) {
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

        if (!tmp->identifier) {
            continue;
        } else if (!strcmp(tmp->identifier, "Param")) {
            if (!(ogex_parse_param(tmp, &attrib, &value))) return 0;
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
        }
    }
    return 1;
}

struct OgexLight* ogex_parse_light_object(const struct OgexContext* context, const struct ODDLStructure* cur) {
    struct OgexLight* light;
    struct ODDLProperty* prop;
    enum OgexLightType type;
    Vec3 color = {1.0, 1.0, 1.0};
    float intensity = 1.0;
    struct OgexAtten atten = {ATTEN_INVERSE_SQUARE, ATTEN_K_DIST, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    unsigned int i;

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

    if (!(light = malloc(sizeof(*light)))) {
        fprintf(stderr, "Warning: LightObject: failed to allocate light\n");
        return 0;
    }
    light->numAtten = 0;
    light->color[0] = 1;
    light->color[1] = 1;
    light->color[2] = 1;
    light->type = OGEX_LIGHT_INFINITE;

    for (i = 0; i < cur->nbStructures; i++) {
        struct ODDLStructure* tmp = cur->structures[i];
        float param;
        char* attrib;

        if (!tmp->identifier) {
            continue;
        } else if (!strcmp(tmp->identifier, "Color")) {
            if (!(ogex_parse_color(tmp, &attrib, color))) return 0;
            if (strcmp(attrib, "light")) {
                fprintf(stderr, "Warning: LightObject: unknown color attribute: %s\n", attrib);
                color[0] = 1; color[1] = 1; color[2] = 1;
            }
        } else if (!strcmp(tmp->identifier, "Param")) {
            if (!(ogex_parse_param(tmp, &attrib, &param))) return 0;
            if (strcmp(attrib, "intensity")) {
                fprintf(stderr, "Warning: LightObject: unknown param attribute: %s\n", attrib);
            } else {
                intensity = param;
            }
        } else if (!strcmp(tmp->identifier, "Texture")) {
            fprintf(stderr, "Warning: LightObject: light textures are not supported yet (but will be Soon(TM))\n");
        } else if (!strcmp(tmp->identifier, "Atten")) {
            if (!(ogex_parse_atten(tmp, &atten))) return 0;
            if (light->numAtten < OGEX_LIGHT_MAX_ATTEN) {
                light->atten[light->numAtten++] = atten;
            } else {
                fprintf(stderr, "Warning: LightObject: exceeded max number of attenuations (OGEX_LIGHT_MAX_ATTEN = %d)\n", OGEX_LIGHT_MAX_ATTEN);
            }
        }
    }
    scale3v(color, intensity);
    if (light->numAtten == 0) {
        light->atten[0] = atten;
        light->numAtten = 1;
    }
    memcpy(light->color, color, sizeof(Vec3));
    light->type = type;
    return light;
}
