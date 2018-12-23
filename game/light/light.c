#include <stdio.h>
#include "light.h"

void light_load_uniforms(GLuint shader, const struct Lights* lights) {
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

