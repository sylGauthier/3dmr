#include <string.h>

#include <3dmr/light/light.h>
#include <3dmr/render/vertex_shader.h>
#include <3dmr/shaders.h>
#include <3dmr/scene/node.h>

#define SHADOW_MAP_W    1024
#define SHADOW_MAP_H    1024

static char lightGlobalInit;
static GLuint depthMapShaders[2];
static GLuint depthMapProgram;
struct UniformBuffer lightCamUBO;

static int load_depth_map_shaders() {
    if (       !(depthMapShaders[0] = vertex_shader_standard(MESH_NORMALS | MESH_TEXCOORDS))
            || !(depthMapShaders[1] = shader_find_compile("depthmap.frag", GL_FRAGMENT_SHADER, &tdmrShaderRootPath, 1, NULL, 0))) {
        fprintf(stderr, "Error: load_depth_map_shaders: can't load shaders\n");
        return 0;
    }
    if (!(depthMapProgram = shader_link(depthMapShaders, 2))) {
        fprintf(stderr, "Error: load_depth_map_shaders: can't link shaders\n");
        return 0;
    }
    return 1;
}

int light_init(struct Lights* lights) {
    unsigned int i;

    memset(lights, 0, sizeof(*lights));
    for (i = 0; i < MAX_DIRECTIONAL_LIGHTS; i++) {
        lights->directional[i].shadow = -1;
    }

    if (!lightGlobalInit) {
        if (!camera_buffer_object_gen(&lightCamUBO)) {
            fprintf(stderr, "Error: light_init: can't gen camera UBO\n");
        } else if (!load_depth_map_shaders()) {
            fprintf(stderr, "Error: light_init: can't create shaders\n");
        } else {
            lightGlobalInit = 1;
            return 1;
        }
        return 0;
    }
    return 1;
}

int light_shadowmap_new(struct Lights* lights) {
    int res = 0;
    struct ShadowMap* map;
    float borderColor[] = {1., 1., 1., 1.};

    for (res = 0; res < MAX_SHADOWMAPS && lights->shadowMaps[res].fbo != 0; res++);
    if (res >= MAX_SHADOWMAPS) return -1;
    map = &lights->shadowMaps[res];

    glGenFramebuffers(1, &map->fbo);
    glGenTextures(1, &map->tex);
    if (!map->fbo || !map->tex) {
        map->fbo = 0;
        map->tex = 0;
        fprintf(stderr, "Error: dirlight_enable_shadow: genBuffers or genTextures failed\n");
        return -1;
    }
    /* setup texture parameters */
    /* bind shadowmap to its global slot, same for all programs */
    glActiveTexture(GL_TEXTURE0 + TEX_SLOT_DIR_SHADOWMAP + res);
    glBindTexture(GL_TEXTURE_2D, map->tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_MAP_W, SHADOW_MAP_H, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    /* bind texture to frame buffer object */
    glBindFramebuffer(GL_FRAMEBUFFER, map->fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, map->tex, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return res;
}

void light_shadowmap_bind(struct Lights* lights) {
    unsigned int i;

    for (i = 0; i < MAX_SHADOWMAPS; i++) {
        struct ShadowMap* map = &lights->shadowMaps[i];

        if (map->fbo && map->tex) {
            glActiveTexture(GL_TEXTURE0 + TEX_SLOT_DIR_SHADOWMAP + i);
            glBindTexture(GL_TEXTURE_2D, map->tex);
        }
    }
}

static void do_render(struct Node* node) {
    unsigned int i;

    if (node->type == NODE_GEOMETRY) {
        glUniformMatrix4fv(glGetUniformLocation(depthMapProgram, "model"), 1, GL_FALSE, &node->model[0][0]);
        vertex_array_render(node->data.geometry->vertexArray);
    }
    for (i = 0; i < node->nbChildren; i++) {
        do_render(node->children[i]);
    }
}

void light_shadowmap_render(struct Lights* lights, int id, struct Node** queue, unsigned int numNodes) {
    unsigned int i;
    GLint viewport[4];
    struct ShadowMap* map = &lights->shadowMaps[id];

    if (!map->fbo || !map->tex) return;

    glGetIntegerv(GL_VIEWPORT, viewport);
    glViewport(0, 0, SHADOW_MAP_W, SHADOW_MAP_H);
    glBindFramebuffer(GL_FRAMEBUFFER, map->fbo);
    glClear(GL_DEPTH_BUFFER_BIT);

    /* bind light camera to "global" camera uniform buffer object */
    uniform_buffer_bind(&lightCamUBO, CAMERA_UBO_BINDING);
    camera_buffer_object_update_view_and_position(&lightCamUBO, MAT_CONST_CAST(map->view));
    camera_buffer_object_update_projection(&lightCamUBO, MAT_CONST_CAST(map->projection));
    uniform_buffer_send(&lightCamUBO);

    glUseProgram(depthMapProgram);
    glCullFace(GL_FRONT);

    for (i = 0; i < numNodes; i++) {
        struct Node* cur = queue[i];

        do_render(cur);
    }
    glCullFace(GL_BACK);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
}
