#include <stdio.h>
#include "light.h"

void light_load_uniforms(GLuint shader, const struct DirectionalLight* directionalLights, unsigned int nbDirectionalLights, const struct LocalLight* localLights, unsigned int nbLocalLights) {
    int i;
    char locName[100];

    glUniform1i(glGetUniformLocation(shader, "nbDirectionalLights"), nbDirectionalLights);
    glUniform1i(glGetUniformLocation(shader, "nbLocalLights"), nbLocalLights);

    for (i = 0; i < nbDirectionalLights; i++) {
        sprintf(locName, "directionalLights[%d].direction", i);
        glUniform3fv(glGetUniformLocation(shader, locName), 1, (float*)directionalLights[i].direction);
        sprintf(locName, "directionalLights[%d].ambient", i);
        glUniform3fv(glGetUniformLocation(shader, locName), 1, (float*)directionalLights[i].ambient);
        sprintf(locName, "directionalLights[%d].diffuse", i);
        glUniform3fv(glGetUniformLocation(shader, locName), 1, (float*)directionalLights[i].diffuse);
        sprintf(locName, "directionalLights[%d].specular", i);
        glUniform3fv(glGetUniformLocation(shader, locName), 1, (float*)directionalLights[i].specular);
    }

    for (i = 0; i < nbLocalLights; i++) {
        sprintf(locName, "localLights[%d].position", i);
        glUniform3fv(glGetUniformLocation(shader, locName), 1, (float*)localLights[i].position);
        sprintf(locName, "localLights[%d].intensity", i);
        glUniform1fv(glGetUniformLocation(shader, locName), 1, &localLights[i].intensity);
        sprintf(locName, "localLights[%d].ambient", i);
        glUniform3fv(glGetUniformLocation(shader, locName), 1, (float*)localLights[i].ambient);
        sprintf(locName, "localLights[%d].diffuse", i);
        glUniform3fv(glGetUniformLocation(shader, locName), 1, (float*)localLights[i].diffuse);
        sprintf(locName, "localLights[%d].specular", i);
        glUniform3fv(glGetUniformLocation(shader, locName), 1, (float*)localLights[i].specular);
    }
}

