#include <game/light/light.h>

#ifndef DEMO_UTIL_LIGHTS_H
#define DEMO_UTIL_LIGHTS_H

int parse_ambient_light(const char* s, struct AmbientLight* dest);
int parse_point_light(const char* s, struct PointLight* dest);
int parse_directional_light(const char* s, struct DirectionalLight* dest);

#endif
