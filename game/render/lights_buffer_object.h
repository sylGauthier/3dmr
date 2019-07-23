#include <GL/glew.h>
#include "../light/light.h"

#ifndef LIGHTS_BUFFER_OBJECT_H
#define LIGHTS_BUFFER_OBJECT_H

#define LIGHTS_UBO_BINDING 1

GLuint lights_buffer_object(void);

void lights_buffer_object_zero_init(GLuint ubo);
void lights_buffer_object_update_ambient(const struct AmbientLight* l, GLuint ubo);
void lights_buffer_object_update_dlight(const struct DirectionalLight* l, unsigned int i, GLuint ubo);
void lights_buffer_object_update_ndlight(unsigned int n, GLuint ubo);
void lights_buffer_object_update_dlights(const struct Lights* l, GLuint ubo);
void lights_buffer_object_update_plight(const struct PointLight* l, unsigned int i, GLuint ubo);
void lights_buffer_object_update_nplight(unsigned int n, GLuint ubo);
void lights_buffer_object_update_plights(const struct Lights* l, GLuint ubo);
void lights_buffer_object_update(const struct Lights* l, GLuint ubo);

#endif
