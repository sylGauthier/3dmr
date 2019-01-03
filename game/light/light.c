#include <stdio.h>
#include "light.h"

void light_init(struct Lights* lights) {
    lights->numDirectionalLights = 0;
    lights->numPointLights = 0;
    lights->ambientLight.color[0] = 0;
    lights->ambientLight.color[1] = 0;
    lights->ambientLight.color[2] = 0;
    lights->ibl.enabled = 0;
}

void light_load_direct_uniforms(GLuint shader, const struct Lights* lights) {
    int i;
    char locName[100];

    glUniform1i(glGetUniformLocation(shader, "numDirectionalLights"), lights->numDirectionalLights);
    glUniform1i(glGetUniformLocation(shader, "numPointLights"), lights->numPointLights);

    for (i = 0; i < lights->numDirectionalLights; i++) {
        const struct DirectionalLight* l = lights->directional + i;
        sprintf(locName, "directionalLights[%d].direction", i);
        glUniform3fv(glGetUniformLocation(shader, locName), 1, (float*)l->direction);
        sprintf(locName, "directionalLights[%d].color", i);
        glUniform3fv(glGetUniformLocation(shader, locName), 1, (float*)l->color);
    }

    for (i = 0; i < lights->numPointLights; i++) {
        const struct PointLight* l = lights->point + i;
        sprintf(locName, "pointLights[%d].position", i);
        glUniform3fv(glGetUniformLocation(shader, locName), 1, (float*)l->position);
        sprintf(locName, "pointLights[%d].color", i);
        glUniform3fv(glGetUniformLocation(shader, locName), 1, (float*)l->color);
        sprintf(locName, "pointLights[%d].radius", i);
        glUniform1fv(glGetUniformLocation(shader, locName), 1, &l->radius);
    }

    glUniform3fv(glGetUniformLocation(shader, "ambientLight.color"), 1, (float*)lights->ambientLight.color);
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
