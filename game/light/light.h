#include <GL/glew.h>
#include <game/math/linear_algebra.h>
#include <game/math/glsl.h>
#include <game/light/ambient_light.h>
#include <game/light/directional_light.h>
#include <game/light/point_light.h>
#include <game/light/ibl.h>

#ifndef LIGHT_H
#define LIGHT_H

struct Lights {
    struct DirectionalLight directional[MAX_DIRECTIONAL_LIGHTS];
    struct PointLight point[MAX_POINT_LIGHTS];
    struct AmbientLight ambientLight;
    struct IBL ibl;
    unsigned int numDirectionalLights, numPointLights;
};

void light_init(struct Lights* lights);
void light_load_ibl_uniforms(GLuint shader, const struct IBL* ibl, unsigned int tex1, unsigned int tex2, unsigned int tex3);

#endif
