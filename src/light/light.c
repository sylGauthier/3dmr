#include <stdio.h>
#include <game/light/light.h>

void light_init(struct Lights* lights) {
    lights->numDirectionalLights = 0;
    lights->numPointLights = 0;
    lights->ambientLight.color[0] = 0;
    lights->ambientLight.color[1] = 0;
    lights->ambientLight.color[2] = 0;
    lights->ibl.enabled = 0;
}

void light_load_ibl_uniforms(GLuint shader, const struct Lights* lights, GLenum tex1, GLenum tex2, GLenum tex3) {
    struct IBL empty = {0};
    const struct IBL* ibl = &lights->ibl;

    if (!ibl->enabled) {
        ibl = &empty; /* It is necessary to bind textures regardless of whether IBL is enabled, otherwise there is a bug with intel cards */
    }
    glUniform1i(glGetUniformLocation(shader, "hasIBL"), !!ibl->enabled);
    glActiveTexture(tex1);
    glBindTexture(GL_TEXTURE_CUBE_MAP, ibl->irradianceMap);
    glUniform1i(glGetUniformLocation(shader, "irradianceMap"), tex1 - GL_TEXTURE0);
    glActiveTexture(tex2);
    glBindTexture(GL_TEXTURE_CUBE_MAP, ibl->specularMap);
    glUniform1i(glGetUniformLocation(shader, "specularMap"), tex2 - GL_TEXTURE0);
    glActiveTexture(tex3);
    glBindTexture(GL_TEXTURE_2D, ibl->specularBrdf);
    glUniform1i(glGetUniformLocation(shader, "specularBrdf"), tex3 - GL_TEXTURE0);

    glUniform1i(glGetUniformLocation(shader, "specularMapNumMipmaps"), ibl->specularMapNumMips);
}
