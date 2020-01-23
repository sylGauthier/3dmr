#include <game/light/light.h>
#include <game/render/camera.h>
#include <game/render/vertex_array.h>

#ifndef MATERIAL_H
#define MATERIAL_H

struct Material {
    void (*vertex_load)(GLuint program, void* params);
    void (*frag_load)(GLuint program, void* params);
    GLuint program;
    GLuint polygonMode;
};

struct Material* material_new(void (*vload)(GLuint, void*), void (*fload)(GLuint, void*), GLuint program, GLuint polygonMode);
struct Material* material_new_from_shaders(const GLuint* shaders, unsigned int numShaders, void (*vload)(GLuint, void*), void (*fload)(GLuint, void*), GLuint polygonMode);
void material_use(const struct Material* material, const void* vertParams, const void* fragParams);
void material_set_matrices(const struct Material* material, Mat4 model, Mat3 inverseNormal);

#endif
