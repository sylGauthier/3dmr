#include <GL/glew.h>
#include <3dmr/shaders.h>
#include <3dmr/light/ibl.h>
#include <3dmr/render/shader.h>

static void params_cubemap_texture(int mip) {
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, mip ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

static void params_2d_texture(void) {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

static int irradiance_map(GLuint envmap, unsigned int size, GLint format, GLuint irradianceMap) {
    GLuint program;
    GLint i;

    if (!(program = shader_find_compile_link_vertfrag("cubemap.vert", "ibl/irradiance_map.frag", &tdmrShaderRootPath, 1, NULL, 0, NULL, 0))) return 0;
    glUseProgram(program);
    glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
    params_cubemap_texture(0);
    for (i = 0; i < 6; i++) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, size, size, 0, GL_RGB, GL_FLOAT, NULL);
    }
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envmap);
    glUniform1i(glGetUniformLocation(program, "envmap"), 0);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, size, size);
    glViewport(0, 0, size, size);
    for (i = 0; i < 6; i++) {
        glUniform1i(glGetUniformLocation(program, "face"), i);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradianceMap, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    glUseProgram(0);
    glDeleteProgram(program);
    return 1;
}

static int specular_map(GLuint envmap, unsigned int size, unsigned int numMipmaps, GLint format, GLuint specularMap) {
    GLuint program;
    GLint i, level;
    float roughness;

    if (!(program = shader_find_compile_link_vertfrag("cubemap.vert", "ibl/specular_map.frag", &tdmrShaderRootPath, 1, NULL, 0, NULL, 0))) return 0;
    glUseProgram(program);
    glBindTexture(GL_TEXTURE_CUBE_MAP, specularMap);
    params_cubemap_texture(1);
    for (i = 0; i < 6; i++) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, size, size, 0, GL_RGB, GL_FLOAT, NULL);
    }
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envmap);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    glUniform1i(glGetUniformLocation(program, "envmap"), 0);
    for (level = 0; level < numMipmaps; level++) {
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, size, size);
        glViewport(0, 0, size, size);
        roughness = (float)level / (float)(numMipmaps - 1);
        glUniform1fv(glGetUniformLocation(program, "roughness"), 1, &roughness);
        for (i = 0; i < 6; i++) {
            glUniform1i(glGetUniformLocation(program, "face"), i);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, specularMap, level);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
        size >>= 1;
    }
    glUseProgram(0);
    glDeleteProgram(program);
    return 1;
}

static int specular_brdf(unsigned int size, GLuint specularBrdf) {
    GLuint program;

    if (!(program = shader_find_compile_link_vertfrag("quad.vert", "ibl/specular_brdf.frag", &tdmrShaderRootPath, 1, NULL, 0, NULL, 0))) return 0;
    glUseProgram(program);
    glBindTexture(GL_TEXTURE_2D, specularBrdf);
    params_2d_texture();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, size, size, 0, GL_RG, GL_FLOAT, 0);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, size, size);
    glViewport(0, 0, size, size);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, specularBrdf, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glUseProgram(0);
    glDeleteProgram(program);
    return 1;
}

static int compute_ibl_(GLuint envmap, unsigned int irrSize, unsigned int spSize, unsigned int spMips, unsigned int spBrdfSize, GLint format, struct IBL* dest) {
    GLuint fbo, rbo, textures[3], empty;
    GLint viewport[4];
    int ret = 0;

    while (glGetError() != GL_NO_ERROR);
    glGenFramebuffers(1, &fbo);
    if (glGetError() == GL_NO_ERROR) {
        glGenRenderbuffers(1, &rbo);
        if (glGetError() == GL_NO_ERROR) {
            glGenTextures(3, textures);
            if (glGetError() == GL_NO_ERROR) {
                glGenVertexArrays(1, &empty);
                if (glGetError() == GL_NO_ERROR) {
                    glGetIntegerv(GL_VIEWPORT, viewport);
                    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
                    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
                    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);
                    glBindVertexArray(empty);
                    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                    ret = irradiance_map(envmap, irrSize, format, textures[0])
                       && specular_map(envmap, spSize, spMips, format, textures[1])
                       && specular_brdf(spBrdfSize, textures[2]);
                    if (ret) {
                        dest->irradianceMap = textures[0];
                        dest->specularMap = textures[1];
                        dest->specularBrdf = textures[2];
                        dest->specularMapNumMips = spMips;
                        dest->enabled = 1;
                    }
                    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
                    glBindVertexArray(0);
                    glDeleteVertexArrays(1, &empty);
                }
                if (!ret) {
                    glDeleteTextures(3, textures);
                }
            }
            glBindRenderbuffer(GL_RENDERBUFFER, 0);
            glDeleteRenderbuffers(1, &rbo);
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDeleteFramebuffers(1, &fbo);
    }

    return ret;
}

int compute_ibl(GLuint envmap, unsigned int irrSize, unsigned int spSize, unsigned int spMips, unsigned int spBrdfSize, struct IBL* dest) {
    return compute_ibl_(envmap, irrSize, spSize, spMips, spBrdfSize, GL_RGB16F, dest);
}

int compute_ibl_32(GLuint envmap, unsigned int irrSize, unsigned int spSize, unsigned int spMips, unsigned int spBrdfSize, struct IBL* dest) {
    return compute_ibl_(envmap, irrSize, spSize, spMips, spBrdfSize, GL_RGB32F, dest);
}

void light_load_ibl_uniforms(GLuint shader, const struct IBL* ibl) {
    struct IBL empty = {0};

    if (!ibl || !ibl->enabled) {
        ibl = &empty; /* It is necessary to bind textures regardless of whether IBL is enabled, otherwise there is a bug with intel cards */
    }

    glUniform1i(glGetUniformLocation(shader, "irradianceMap"), TEX_SLOT_IBL_IRRADIANCE);
    glUniform1i(glGetUniformLocation(shader, "specularMap"), TEX_SLOT_IBL_SPECULAR);
    glUniform1i(glGetUniformLocation(shader, "specularBrdf"), TEX_SLOT_IBL_SPECBRDF);

    glUniform1i(glGetUniformLocation(shader, "hasIBL"), !!ibl->enabled);
    glActiveTexture(GL_TEXTURE0 + TEX_SLOT_IBL_IRRADIANCE);
    glBindTexture(GL_TEXTURE_CUBE_MAP, ibl->irradianceMap);
    glActiveTexture(GL_TEXTURE0 + TEX_SLOT_IBL_SPECULAR);
    glBindTexture(GL_TEXTURE_CUBE_MAP, ibl->specularMap);
    glActiveTexture(GL_TEXTURE0 + TEX_SLOT_IBL_SPECBRDF);
    glBindTexture(GL_TEXTURE_2D, ibl->specularBrdf);

    glUniform1i(glGetUniformLocation(shader, "specularMapNumMipmaps"), ibl->specularMapNumMips);
}
