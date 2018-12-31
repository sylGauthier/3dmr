#include "args.h"
#include "lights.h"

int parse_ambient_light(const char* s, struct AmbientLight* dest) {
    return parse_vec3(s, (char**)&s, dest->color) && !*s;
}

int parse_point_light(const char* s, struct PointLight* dest) {
    return parse_vec3(s, (char**)&s, dest->position) && *s++ == ';'
        && parse_vec3(s, (char**)&s, dest->color) && *s++ == ';'
        && parse_float(s, (char**)&s, &dest->radius) && !*s
        && dest->radius > 0.0;
}

int parse_directional_light(const char* s, struct DirectionalLight* dest) {
    if (parse_vec3(s, (char**)&s, dest->direction) && *s++ == ';'
     && parse_vec3(s, (char**)&s, dest->color) && !*s) {
        normalize3(dest->direction);
        return 1;
    }
    return 0;
}
