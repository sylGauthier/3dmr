#include <game/light/light.h>
#include <game/render/camera.h>

#ifndef MATERIAL_H
#define MATERIAL_H

struct Material {
    void (*load)(const struct Material*);
    void* params;
    GLuint program;
    GLuint polygonMode;
};

struct Material* material_new(void (*load)(const struct Material*), void* params, GLuint program, GLuint polygonMode);

#endif
