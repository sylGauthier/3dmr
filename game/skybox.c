#include <stdlib.h>
#include <game/asset_manager.h>
#include <game/material/shaders.h>
#include <game/mesh/box.h>
#include <game/render/globject.h>
#include <game/render/texture.h>
#include <game/img/png.h>

struct SkyboxMaterial {
    struct Material material;
    GLuint texture;
};

static int load_face(const char* path, GLenum target) {
    unsigned char* buffer;
    unsigned int width, height;
    int alpha;

    if (png_read(path, 4, &width, &height, &alpha, 0, &buffer)) {
        glTexImage2D(target, 0, alpha ? GL_RGBA : GL_RGB, width, height, 0, alpha ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, buffer);
        free(buffer);
        return 1;
    }
    return 0;
}

GLuint skybox_load_texture(const char* left, const char* right, const char* bottom, const char* top, const char* back, const char* front) {
    GLuint res;

    glGenTextures(1, &res);
    if (!res) {
        return 0;
    }
    glBindTexture(GL_TEXTURE_CUBE_MAP, res);
    if (load_face(left, GL_TEXTURE_CUBE_MAP_NEGATIVE_X)
     && load_face(right, GL_TEXTURE_CUBE_MAP_POSITIVE_X)
     && load_face(bottom, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y)
     && load_face(top, GL_TEXTURE_CUBE_MAP_POSITIVE_Y)
     && load_face(back, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z)
     && load_face(front, GL_TEXTURE_CUBE_MAP_POSITIVE_Z)) {
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        return res;
    }
    glDeleteTextures(1, &res);
    return 0;
}

static void skybox_prerender(const struct Material* material, const struct Camera* camera, const struct Lights* lights) {
    glBindTexture(GL_TEXTURE_CUBE_MAP, ((const struct SkyboxMaterial*)material)->texture);
}

static void skybox_postrender(const struct Material* material, const struct Camera* camera, const struct Lights* lights) {
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

int skybox_create(GLuint texture, float size, struct GLObject* skybox) {
    struct Mesh box;
    struct SkyboxMaterial* mat;

    if (!game_shaders[SHADER_SKYBOX]) {
        if (!(game_shaders[SHADER_SKYBOX] = asset_manager_load_shader("shaders/skybox.vert", "shaders/skybox.frag"))) {
            return 0;
        }
    }
    if (make_box(&box, size, size, size)) {
        skybox->vertexArray = vertex_array_new(&box);
        mesh_free(&box);
    } else {
        skybox->vertexArray = NULL;
    }
    if (!skybox->vertexArray) {
        return 0;
    }
    if (!(mat = malloc(sizeof(struct SkyboxMaterial)))) {
        vertex_array_free(skybox->vertexArray);
        return 0;
    }
    skybox->material = (struct Material*)mat;
    mat->material.prerender = skybox_prerender;
    mat->material.postrender = skybox_postrender;
    mat->material.shader = game_shaders[SHADER_SKYBOX];
    mat->material.polygonMode = GL_FILL;
    mat->texture = texture;

    return 1;
}

void skybox_destroy(struct GLObject* skybox) {
    vertex_array_free(skybox->vertexArray);
    free(skybox->material);
}

struct GLObject* skybox_new(GLuint texture, float size) {
    struct GLObject* res;

    if ((res = malloc(sizeof(*res)))) {
        if (skybox_create(texture, size, res)) {
            return res;
        }
        free(res);
    }
    return NULL;
}

void skybox_free(struct GLObject* skybox) {
    if (skybox) {
        skybox_destroy(skybox);
        free(skybox);
    }
}

void skybox_render(struct GLObject* skybox, const struct Camera* camera) {
    Mat4 model;
    Mat3 inv;
    globject_render(skybox, camera, NULL, model, inv);
}
