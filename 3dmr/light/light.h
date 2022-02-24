#include <GL/glew.h>
#include <3dmr/math/linear_algebra.h>
#include <3dmr/render/camera_buffer_object.h>
#include <3dmr/light/ambient_light.h>
#include <3dmr/light/directional_light.h>
#include <3dmr/light/point_light.h>
#include <3dmr/light/spot_light.h>
#include <3dmr/light/ibl.h>
#include <shaders/light/shadowmap.h>

#ifndef TDMR_LIGHT_H
#define TDMR_LIGHT_H

struct ShadowMap {
    GLuint fbo;
    GLuint tex;
    Mat4 view, projection;
    unsigned int width, height;
};

struct Lights {
    struct DirectionalLight directional[MAX_DIRECTIONAL_LIGHTS];
    struct PointLight point[MAX_POINT_LIGHTS];
    struct SpotLight spot[MAX_SPOT_LIGHTS];
    struct AmbientLight ambientLight;
    unsigned int numDirectionalLights, numPointLights, numSpotLights;

    struct ShadowMap shadowMaps[MAX_SHADOWMAPS];
};

struct Node;

int light_init(struct Lights* lights);

int light_shadowmap_new(struct Lights* lights, unsigned int w, unsigned int h);
void light_shadowmap_bind(struct Lights* lights);
void light_shadowmap_render_start(struct Lights* lights, int id, GLint viewport[4]);
void light_shadowmap_render_end(struct Lights* lights, GLint viewport[4]);
void light_shadowmap_render_node(struct Node* node);
void light_shadowmap_render(struct Lights* lights, int id, struct Node** queue, unsigned int numNodes);

#endif
