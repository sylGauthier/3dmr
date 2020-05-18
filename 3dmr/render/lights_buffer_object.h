#include <3dmr/render/uniform_buffer.h>
#include "../light/light.h"

#ifndef TDMR_LIGHTS_BUFFER_OBJECT_H
#define TDMR_LIGHTS_BUFFER_OBJECT_H

int lights_buffer_object_gen(struct UniformBuffer* dest);
struct UniformBuffer* lights_buffer_object_new(void);

void lights_buffer_object_zero_init(struct UniformBuffer* u);

void lights_buffer_object_update_ambient(struct UniformBuffer* u, const struct AmbientLight* l);

void lights_buffer_object_update_dlight(struct UniformBuffer* u, const struct DirectionalLight* l, unsigned int i);
void lights_buffer_object_update_ndlight(struct UniformBuffer* u, unsigned int n);
void lights_buffer_object_update_dlights(struct UniformBuffer* u, const struct Lights* l);

void lights_buffer_object_update_plight(struct UniformBuffer* u, const struct PointLight* l, unsigned int i);
void lights_buffer_object_update_nplight(struct UniformBuffer* u, unsigned int n);
void lights_buffer_object_update_plights(struct UniformBuffer* u, const struct Lights* l);

void lights_buffer_object_update_slight(struct UniformBuffer* u, const struct SpotLight* l, unsigned int i);
void lights_buffer_object_update_nslight(struct UniformBuffer* u, unsigned int n);
void lights_buffer_object_update_slights(struct UniformBuffer* u, const struct Lights* l);

void lights_buffer_object_update(struct UniformBuffer* u, const struct Lights* l);

#endif
