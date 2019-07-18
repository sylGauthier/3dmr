#include <stdarg.h>
#include <string.h>
#include <game/material/param.h>

void material_param_send_texture(GLuint shader, GLuint tex, const char* name, unsigned int* texSlot) {
    GLint uniform = glGetUniformLocation(shader, name);
    GLint texId = GL_TEXTURE0 + *texSlot;
    glActiveTexture(texId);
    glBindTexture(GL_TEXTURE_2D, tex);
    glUniform1i(uniform, *texSlot);
    (*texSlot)++;
}

void material_param_set_float_constant(struct MatParamFloat* p, float v) {
    p->mode = MAT_PARAM_CONSTANT;
    p->value.constant = v;
}

void material_param_set_float_shared(struct MatParamFloat* p, float* v) {
    p->mode = MAT_PARAM_SHARED;
    p->value.shared = v;
}

void material_param_set_float_texture(struct MatParamFloat* p, GLuint v) {
    p->mode = MAT_PARAM_TEXTURED;
    p->value.texture = v;
}

void material_param_send_float(GLuint shader, const struct MatParamFloat* param, const char* name, unsigned int* texSlot) {
    GLint uniform = glGetUniformLocation(shader, name);
    switch (param->mode) {
        case MAT_PARAM_CONSTANT:
            glUniform1fv(uniform, 1, &param->value.constant);
            break;
        case MAT_PARAM_SHARED:
            glUniform1fv(uniform, 1, param->value.shared);
            break;
        case MAT_PARAM_TEXTURED:
            {
                GLint texId = GL_TEXTURE0 + *texSlot;
                glActiveTexture(texId);
                glBindTexture(GL_TEXTURE_2D, param->value.texture);
                glUniform1i(uniform, *texSlot);
                (*texSlot)++;
            }
            break;
    }
}

void material_param_set_vec3_constant(struct MatParamVec3* p, const Vec3 v) {
    p->mode = MAT_PARAM_CONSTANT;
    memcpy(p->value.constant, v, sizeof(Vec3));
}

void material_param_set_vec3_elems(struct MatParamVec3* p, float x, float y, float z) {
    p->mode = MAT_PARAM_CONSTANT;
    p->value.constant[0] = x;
    p->value.constant[1] = y;
    p->value.constant[2] = z;
}

void material_param_set_vec3_shared(struct MatParamVec3* p, float* v) {
    p->mode = MAT_PARAM_SHARED;
    p->value.shared = v;
}

void material_param_set_vec3_texture(struct MatParamVec3* p, GLuint v) {
    p->mode = MAT_PARAM_TEXTURED;
    p->value.texture = v;
}

void material_param_send_vec3(GLuint shader, const struct MatParamVec3* param, const char* name, unsigned int* texSlot) {
    GLint uniform = glGetUniformLocation(shader, name);
    switch (param->mode) {
        case MAT_PARAM_CONSTANT:
            glUniform3fv(uniform, 1, param->value.constant);
            break;
        case MAT_PARAM_SHARED:
            glUniform3fv(uniform, 1, param->value.shared);
            break;
        case MAT_PARAM_TEXTURED:
            {
                GLint texId = GL_TEXTURE0 + *texSlot;
                glActiveTexture(texId);
                glBindTexture(GL_TEXTURE_2D, param->value.texture);
                glUniform1i(uniform, *texSlot);
                (*texSlot)++;
            }
            break;
    }
}
