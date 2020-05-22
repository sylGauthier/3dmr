#include <stdlib.h>
#include <3dmr/shaders.h>
#include <3dmr/mesh/box.h>
#include <3dmr/render/shader.h>
#include <3dmr/render/texture.h>
#include <3dmr/img/hdr.h>
#include <3dmr/img/png.h>
#include <3dmr/skybox.h>

static void texture_params_cubemap(void) {
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

static void texture_params_2d(void) {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

static int load_face(const char* path, GLenum target) {
    unsigned char* ldr;
    float* hdr;
    unsigned int width, height, channels;

    if (png_read(path, 4, &width, &height, &channels, 3, 0, &ldr)) {
        if ((hdr = ldr_to_hdr(ldr, width, height, 4, 0, 2.2f, 1.0f))) {
            glTexImage2D(target, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, hdr);
            free(hdr);
        }
        free(ldr);
        return 1;
    }
    return 0;
}

GLuint skybox_load_texture_png_6faces(const char* left, const char* right, const char* bottom, const char* top, const char* back, const char* front) {
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
        texture_params_cubemap();
        return res;
    }
    glDeleteTextures(1, &res);
    return 0;
}

static GLuint skybox_load_texture_hdr_equirect_(const char* path, unsigned int cubeFaceSize, GLint format) {
    GLuint tex[2], program, fbo, rbo, empty;
    GLint viewport[4];
    float* data;
    unsigned int width, height;
    GLint i;
    int ok = 0;

    while (glGetError() != GL_NO_ERROR);
    glGenFramebuffers(1, &fbo);
    if (glGetError() == GL_NO_ERROR) {
        glGenRenderbuffers(1, &rbo);
        if (glGetError() == GL_NO_ERROR) {
            glGenTextures(2, tex);
            if (glGetError() == GL_NO_ERROR) {
                glGenVertexArrays(1, &empty);
                if (glGetError() == GL_NO_ERROR) {
                    glGetIntegerv(GL_VIEWPORT, viewport);
                    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
                    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
                    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);
                    glBindVertexArray(empty);
                    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                    if (hdr_read(path, 4, &width, &height, &data)) {
                        if ((program = shader_find_compile_link_vertfrag("cubemap.vert", "equirect_to_cubemap.frag", &tdmrShaderRootPath, 1, NULL, 0, NULL, 0))) {
                            glUseProgram(program);
                            glBindTexture(GL_TEXTURE_CUBE_MAP, tex[1]);
                            texture_params_cubemap();
                            for (i = 0; i < 6; i++) {
                                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, cubeFaceSize, cubeFaceSize, 0, GL_RGB, GL_FLOAT, NULL);
                            }
                            glActiveTexture(GL_TEXTURE0);
                            glBindTexture(GL_TEXTURE_2D, tex[0]);
                            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, GL_RGB, GL_FLOAT, data);
                            texture_params_2d();
                            glUniform1i(glGetUniformLocation(program, "source"), 0);
                            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, cubeFaceSize, cubeFaceSize);
                            glViewport(0, 0, cubeFaceSize, cubeFaceSize);
                            for (i = 0; i < 6; i++) {
                                glUniform1i(glGetUniformLocation(program, "face"), i);
                                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, tex[1], 0);
                                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                                glDrawArrays(GL_TRIANGLES, 0, 6);
                            }
                            glUseProgram(0);
                            glDeleteProgram(program);
                            ok = 1;
                        }
                        free(data);
                    }
                    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
                    glBindVertexArray(0);
                    glDeleteVertexArrays(1, &empty);
                }
                glDeleteTextures(1 + !ok, tex);
            }
            glBindRenderbuffer(GL_RENDERBUFFER, 0);
            glDeleteRenderbuffers(1, &rbo);
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDeleteFramebuffers(1, &fbo);
    }

    return ok ? tex[1] : 0;
}

GLuint skybox_load_texture_hdr_equirect(const char* path, unsigned int cubeFaceSize) {
    return skybox_load_texture_hdr_equirect_(path, cubeFaceSize, GL_RGB16F);
}

GLuint skybox_load_texture_hdr_equirect_32(const char* path, unsigned int cubeFaceSize) {
    return skybox_load_texture_hdr_equirect_(path, cubeFaceSize, GL_RGB32F);
}

static void skybox_load(GLuint program, void* param) {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, *(GLuint*)param);
    glUniform1i(glGetUniformLocation(program, "tex"), 0);
}

int skybox_create(GLuint texture, struct Skybox* skybox) {
    struct Mesh box;
    GLuint prog;

    if (!(prog = shader_find_compile_link_vertfrag("skybox.vert", "skybox.frag", &tdmrShaderRootPath, 1, NULL, 0, NULL, 0))) return 0;
    if (make_box(&box, 1, 1, 1)) {
        vertex_array_gen(&box, &skybox->vertexArray);
        mesh_free(&box);
    }
    skybox->material.load = skybox_load;
    skybox->material.params = &skybox->texture;
    skybox->material.program = prog;
    skybox->material.polygonMode = GL_FILL;
    skybox->texture = texture;

    return 1;
}

void skybox_destroy(struct Skybox* skybox) {
    vertex_array_del(&skybox->vertexArray);
    glDeleteProgram(skybox->material.program);
}

struct Skybox* skybox_new(GLuint texture) {
    struct Skybox* res;

    if ((res = malloc(sizeof(*res)))) {
        if (skybox_create(texture, res)) {
            return res;
        }
        free(res);
    }
    return NULL;
}

void skybox_free(struct Skybox* skybox) {
    if (skybox) {
        skybox_destroy(skybox);
        free(skybox);
    }
}

void skybox_render(struct Skybox* skybox) {
    GLint dfunc;
    glGetIntegerv(GL_DEPTH_FUNC, &dfunc);
    glDepthFunc(GL_LEQUAL);
    material_use(&skybox->material);
    vertex_array_render(&skybox->vertexArray);
    glDepthFunc(dfunc);
}
