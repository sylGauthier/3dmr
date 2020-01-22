#include <game/light/light.h>
#include <game/render/camera.h>
#include <game/render/vertex_array.h>

#ifndef MATERIAL_H
#define MATERIAL_H

struct Material {
    void (*vertex_load)(void* params);
    void (*frag_load)(GLuint program, void* params);
    void *vparams, *fparams;
    GLuint program;
    GLuint polygonMode;
};

struct Material* material_new(void (*vload)(void*), void (*fload)(GLuint, void*), const void* vparams, const void* fparams, GLuint program, GLuint polygonMode);
struct Material* material_new_from_shaders(const GLuint* shaders, unsigned int numShaders, void (*vload)(void*), void (*fload)(GLuint, void*), const void* vparams, const void* fparams, GLuint polygonMode);
void material_use(const struct Material* material);
void material_set_matrices(const struct Material* material, Mat4 model, Mat3 inverseNormal);

#endif
