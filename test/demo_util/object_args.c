#include <stdio.h>
#include <string.h>
#include <game/math/linear_algebra.h>
#include "args.h"
#include "object_args.h"

void usage_object_args(void) {
    printf(
            "Object arguments:\n"
            "  t(x,y,z)                             translate the object\n"
            "  r(x,y,z,angle)                       rotate the object\n"
            "\n"
          );
}

int parse_object_args(const char* s, char** e, struct Node* object) {
    if (!strncmp(s, "t(", 2)) {
        Vec3 t;
        if (parse_vec3(s + 2, (char**)&s, t) && *s++ == ')') {
            node_translate(object, t);
            if (e) {
                *e = (char*)s;
            }
            return 1;
        }
    } else if (!strncmp(s, "r(", 2)) {
        Vec4 r;
        if (parse_vec4(s + 2, (char**)&s, r) && *s++ == ')') {
            node_rotate(object, r, r[3]);
            if (e) {
                *e = (char*)s;
            }
            return 1;
        }
    }
    return 0;
}
