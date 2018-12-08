#include <stdio.h>
#include "light.h"

void light_load_uniforms(GLuint shader, const struct Lights* lights) {
    int i;
    char locName[100];

    glUniform1i(glGetUniformLocation(shader, "nbDirectionalLights"), lights->numDirectionalLights);
    glUniform1i(glGetUniformLocation(shader, "nbLocalLights"), lights->numPointLights);

    for (i = 0; i < lights->numDirectionalLights; i++) {
        const struct DirectionalLight* l = lights->directional + i;
        sprintf(locName, "directionalLights[%d].direction", i);
        glUniform3fv(glGetUniformLocation(shader, locName), 1, (float*)l->direction);
        sprintf(locName, "directionalLights[%d].ambient", i);
        glUniform3fv(glGetUniformLocation(shader, locName), 1, (float*)l->ambient);
        sprintf(locName, "directionalLights[%d].diffuse", i);
        glUniform3fv(glGetUniformLocation(shader, locName), 1, (float*)l->diffuse);
        sprintf(locName, "directionalLights[%d].specular", i);
        glUniform3fv(glGetUniformLocation(shader, locName), 1, (float*)l->specular);
    }

    for (i = 0; i < lights->numPointLights; i++) {
        const struct PointLight* l = lights->point + i;
        sprintf(locName, "localLights[%d].position", i);
        glUniform3fv(glGetUniformLocation(shader, locName), 1, (float*)l->position);
        sprintf(locName, "localLights[%d].intensity", i);
        glUniform1fv(glGetUniformLocation(shader, locName), 1, &l->intensity);
        sprintf(locName, "localLights[%d].decay", i);
        glUniform1fv(glGetUniformLocation(shader, locName), 1, &l->decay);
        sprintf(locName, "localLights[%d].ambient", i);
        glUniform3fv(glGetUniformLocation(shader, locName), 1, (float*)l->ambient);
        sprintf(locName, "localLights[%d].diffuse", i);
        glUniform3fv(glGetUniformLocation(shader, locName), 1, (float*)l->diffuse);
        sprintf(locName, "localLights[%d].specular", i);
        glUniform3fv(glGetUniformLocation(shader, locName), 1, (float*)l->specular);
    }
}

