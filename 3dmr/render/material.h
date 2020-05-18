#include <3dmr/light/light.h>
#include <3dmr/render/camera.h>
#include <3dmr/render/vertex_array.h>

#ifndef TDMR_RENDER_MATERIAL_H
#define TDMR_RENDER_MATERIAL_H

struct Material {
    void (*load)(GLuint program, void* params);
    void* params;
    GLuint program;
    GLuint polygonMode;
};

struct Material* material_new(void (*load)(GLuint, void*), void* params, GLuint program, GLuint polygonMode);
struct Material* material_new_from_shaders(const GLuint* shaders, unsigned int numShaders, void (*load)(GLuint, void*), void* params, GLuint polygonMode);
void material_use(const struct Material* material);
void material_set_matrices(const struct Material* material, Mat4 model, Mat3 inverseNormal);

#endif
