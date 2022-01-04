#include <GL/glew.h>
#include <3dmr/math/linear_algebra.h>
#include <3dmr/render/camera_buffer_object.h>
#include <3dmr/light/ambient_light.h>
#include <3dmr/light/directional_light.h>
#include <3dmr/light/point_light.h>
#include <3dmr/light/spot_light.h>
#include <3dmr/light/ibl.h>

#ifndef TDMR_LIGHT_H
#define TDMR_LIGHT_H

struct FBTexture {
    GLuint fbo;
    GLuint tex;
};

struct Lights {
    struct DirectionalLight directional[MAX_DIRECTIONAL_LIGHTS];
    struct PointLight point[MAX_POINT_LIGHTS];
    struct SpotLight spot[MAX_SPOT_LIGHTS];
    struct AmbientLight ambientLight;
    unsigned int numDirectionalLights, numPointLights, numSpotLights;

    struct FBTexture directionalLightDepthMap[MAX_DIRECTIONAL_LIGHTS];
    unsigned int numDLDepthMaps;
};

struct Node;

int light_init(struct Lights* lights);

int dirlight_enable_shadow(struct Lights* lights, unsigned int id);
void dirlight_render_depthmap(struct Lights* lights, unsigned int id, struct Node** queue, unsigned int numNodes);

#endif
