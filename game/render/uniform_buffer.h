#include <GL/glew.h>

#ifndef UNIFORM_BUFFER_H
#define UNIFORM_BUFFER_H

struct UniformBuffer {
    void* cache;
    unsigned int dStart, dEnd, size;
    GLuint ubo;
};

int uniform_buffer_gen(unsigned int size, struct UniformBuffer* dest);
void uniform_buffer_del(struct UniformBuffer* u);

struct UniformBuffer* uniform_buffer_new(unsigned int size);
void uniform_buffer_free(struct UniformBuffer* u);

void uniform_buffer_invalidate(struct UniformBuffer* u, unsigned int offset, unsigned int size);
void uniform_buffer_update(struct UniformBuffer* u, unsigned int offset, unsigned int size, const void* data);
void uniform_buffer_send(struct UniformBuffer* u);

#endif
