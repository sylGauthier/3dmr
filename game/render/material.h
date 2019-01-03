#include <game/light/light.h>
#include <game/render/camera.h>

#ifndef MATERIAL_H
#define MATERIAL_H

struct Material {
    void (*load)(const struct Material*, const struct Camera*, const struct Lights*);
    GLuint shader;
    GLuint polygonMode;
};

#endif
