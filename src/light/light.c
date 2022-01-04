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
    lights->numDirectionalLights = 0;
    lights->numPointLights = 0;
    lights->numSpotLights = 0;
    lights->ambientLight.color[0] = 0;
    lights->ambientLight.color[1] = 0;
    lights->ambientLight.color[2] = 0;

    lights->numDLDepthMaps = 0;

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

int dirlight_enable_shadow(struct Lights* lights, unsigned int id) {
    struct DirectionalLight* dl = &lights->directional[id];
    struct FBTexture* fbtex = &lights->directionalLightDepthMap[id];

    if (dl->shadow) return 1; /* already enabled */

    dl->depthMapID = id;
    glGenFramebuffers(1, &fbtex->fbo);
    glGenTextures(1, &fbtex->tex);
    if (!fbtex->fbo || !fbtex->tex) {
        fprintf(stderr, "Error: dirlight_enable_shadow: genBuffers or genTextures failed\n");
        return 0;
    }
    /* setup texture parameters */
    glBindTexture(GL_TEXTURE_2D, fbtex->tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_MAP_W, SHADOW_MAP_H, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    /* bind texture to frame buffer object */
    glBindFramebuffer(GL_FRAMEBUFFER, fbtex->fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, fbtex->tex, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    dl->shadow = 1;
    return 1;
}

void dirlight_render_depthmap(struct Lights* lights, unsigned int id, struct Node** queue, unsigned int numNodes) {
    unsigned int i;
    struct DirectionalLight* dl = &lights->directional[id];
    GLint viewport[4];

    if (!dl->shadow) return;

    glGetIntegerv(GL_VIEWPORT, viewport);
    glViewport(0, 0, SHADOW_MAP_W, SHADOW_MAP_H);
    glBindFramebuffer(GL_FRAMEBUFFER, lights->directionalLightDepthMap[id].fbo);
    glClear(GL_DEPTH_BUFFER_BIT);

    /* bind light camera to "global" camera uniform buffer object */
    uniform_buffer_bind(&lightCamUBO, CAMERA_UBO_BINDING);
    camera_buffer_object_update_view_and_position(&lightCamUBO, MAT_CONST_CAST(dl->view));
    camera_buffer_object_update_projection(&lightCamUBO, MAT_CONST_CAST(dl->projection));
    uniform_buffer_send(&lightCamUBO);

    glUseProgram(depthMapProgram);

    for (i = 0; i < numNodes; i++) {
        struct Node* cur = queue[i];

        if (cur->type == NODE_GEOMETRY) {
            glUniformMatrix4fv(glGetUniformLocation(depthMapProgram, "model"), 1, GL_FALSE, &cur->model[0][0]);
            vertex_array_render(cur->data.geometry->vertexArray);
        }
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
}
