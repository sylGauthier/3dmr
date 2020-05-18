#include <liboddl/liboddl.h>
#include "context.h"

#ifndef TDMR_OPENGEX_LIGHT_OBJECT_H
#define TDMR_OPENGEX_LIGHT_OBJECT_H

#define OGEX_LIGHT_MAX_ATTEN 5

enum OgexLightType {
    OGEX_LIGHT_INFINITE,
    OGEX_LIGHT_POINT,
    OGEX_LIGHT_SPOT
};

struct OgexAtten {
    enum OgexAttenType {
        ATTEN_LINEAR,
        ATTEN_SMOOTH,
        ATTEN_INVERSE,
        ATTEN_INVERSE_SQUARE
    } type;
    enum OgexAttenKind {
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

struct OgexLight {
    struct OgexAtten atten[OGEX_LIGHT_MAX_ATTEN];
    unsigned int numAtten;
    Vec3 color;
    enum OgexLightType type;
};

struct OgexLight* ogex_parse_light_object(const struct OgexContext* context, const struct ODDLStructure* cur);

#endif
