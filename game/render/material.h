#include <game/light/light.h>
#include <game/render/camera.h>
#include <game/render/vertex_array.h>

#ifndef MATERIAL_H
#define MATERIAL_H

struct Material {
    void (*vertex_load)(const struct VertexArray* va);
    void (*load)(GLuint program, void* params);
    void* params;
    GLuint program;
    GLuint polygonMode;
};

struct Material* material_new(void (*vertex_load)(const struct VertexArray* va), void (*load)(GLuint program, void* params), const void* params, GLuint program, GLuint polygonMode);
struct Material* material_new_from_shaders(const GLuint* shaders, unsigned int numShaders, void (*vertex_load)(const struct VertexArray* va), void (*load)(GLuint program, void* params), const void* params, GLuint polygonMode);
void material_use(const struct Material* material, const struct VertexArray* va);
void material_set_matrices(const struct Material* material, Mat4 model, Mat3 inverseNormal);

#endif
