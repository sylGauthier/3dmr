#include <stdlib.h>
#include <game/render/material.h>

struct Material* material_new(void (*load)(const struct Material*), void* params, GLuint program, GLuint polygonMode) {
    struct Material* m;
    if ((m = malloc(sizeof(*m)))) {
        m->load = load;
        m->params = params;
        m->program = program;
        m->polygonMode = polygonMode;
    }
    return m;
}
