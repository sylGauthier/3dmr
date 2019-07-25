#include <stdlib.h>
#include <game/mesh/box.h>
#include <game/render/camera_buffer_object.h>
#include <game/render/texture.h>
#include <game/render/viewer.h>
#include <game/img/hdr.h>
#include <game/img/png.h>
#include <game/skybox.h>
#include "material/programs.h"

static unsigned int progid = -1;

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
    unsigned int width, height;
    int alpha;

    if (png_read(path, 4, &width, &height, &alpha, 0, &ldr)) {
        if ((hdr = ldr_to_hdr(ldr, width, height, 4, alpha, 2.2f, 1.0f))) {
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

GLuint skybox_load_texture_hdr_equirect(const char* path, unsigned int cubeFaceSize) {
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
                        if ((program = game_load_shader("cubemap.vert", "equirect_to_cubemap.frag", NULL, 0))) {
                            glUseProgram(program);
                            glBindTexture(GL_TEXTURE_CUBE_MAP, tex[1]);
                            texture_params_cubemap();
                            for (i = 0; i < 6; i++) {
                                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, cubeFaceSize, cubeFaceSize, 0, GL_RGB, GL_FLOAT, NULL);
                            }
                            glActiveTexture(GL_TEXTURE0);
                            glBindTexture(GL_TEXTURE_2D, tex[0]);
                            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data);
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

static void skybox_load(const struct Material* material) {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, ((const struct Skybox*)material)->texture);
    glUniform1i(glGetUniformLocation(material->program, "tex"), 0);
}

int skybox_create(GLuint texture, struct Skybox* skybox) {
    struct Mesh box;
    struct Viewer* currentViewer;
    GLuint prog;

    if (progid == ((unsigned int)-1)) {
        if ((progid = viewer_register_program_id()) == ((unsigned int)-1)) {
            return 0;
        }
    }
    if (!(currentViewer = viewer_get_current())) {
        return 0;
    }
    if (!(prog = viewer_get_program(currentViewer, progid))) {
        if (!(prog = game_load_shader("skybox.vert", "skybox.frag", NULL, 0))) {
            return 0;
        }
        glUniformBlockBinding(prog, glGetUniformBlockIndex(prog, "Camera"), CAMERA_UBO_BINDING);
        if (!viewer_set_program(currentViewer, progid, prog)) {
            glDeleteProgram(prog);
            return 0;
        }
    }
    if (make_box(&box, 1, 1, 1)) {
        vertex_array_gen(&box, &skybox->vertexArray);
        mesh_free(&box);
    }
    skybox->material.load = skybox_load;
    skybox->material.program = prog;
    skybox->material.polygonMode = GL_FILL;
    skybox->texture = texture;

    return 1;
}

void skybox_destroy(struct Skybox* skybox) {
    vertex_array_del(&skybox->vertexArray);
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
    Mat4 model;
    Mat3 inv;
    GLint dfunc;
    glGetIntegerv(GL_DEPTH_FUNC, &dfunc);
    glDepthFunc(GL_LEQUAL);
    vertex_array_render(&skybox->vertexArray, &skybox->material, model, inv);
    glDepthFunc(dfunc);
}
