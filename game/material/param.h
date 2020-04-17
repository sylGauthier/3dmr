#include <GL/glew.h>
#include <game/math/linear_algebra.h>

#ifndef GAME_MATERIAL_PARAM_H
#define GAME_MATERIAL_PARAM_H

enum MatParamMode {
    MAT_PARAM_CONSTANT,
    MAT_PARAM_SHARED,
    MAT_PARAM_TEXTURED
};

struct MatParamFloat {
    union MatParamFloatValue {
        float constant;
        float* shared;
        GLuint texture;
    } value;
    enum MatParamMode mode;
};

struct MatParamVec3 {
    union MatParamVec3Value {
        Vec3 constant;
        float* shared;
        GLuint texture;
    } value;
    enum MatParamMode mode;
};

void material_param_send_texture(GLuint shader, GLuint tex, const char* name, unsigned int* texSlot);

void material_param_set_float_constant(struct MatParamFloat* p, float v);
void material_param_set_float_shared(struct MatParamFloat* p, float* v);
void material_param_set_float_texture(struct MatParamFloat* p, GLuint v);
void material_param_send_float(GLuint shader, const struct MatParamFloat* param, const char* name, unsigned int* texSlot);

void material_param_set_vec3_constant(struct MatParamVec3* p, const Vec3 v);
void material_param_set_vec3_elems(struct MatParamVec3* p, float x, float y, float z);
void material_param_set_vec3_shared(struct MatParamVec3* p, float* v);
void material_param_set_vec3_texture(struct MatParamVec3* p, GLuint v);
void material_param_send_vec3(GLuint shader, const struct MatParamVec3* param, const char* name, unsigned int* texSlot);

#endif
