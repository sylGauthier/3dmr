#include "phong.h"

void phong_load_material_uniform(GLuint shader, const struct PhongMaterial* material) {
    glUniform3fv(glGetUniformLocation(shader, "material.ambient"), 1, (float*)material->ambient);
    glUniform3fv(glGetUniformLocation(shader, "material.diffuse"), 1, (float*)material->diffuse);
    glUniform3fv(glGetUniformLocation(shader, "material.specular"), 1, (float*)material->specular);
    glUniform1fv(glGetUniformLocation(shader, "material.shininess"), 1, &material->shininess);
}
