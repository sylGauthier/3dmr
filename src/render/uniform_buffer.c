#include <stdlib.h>
#include <string.h>
#include <3dmr/render/uniform_buffer.h>

int uniform_buffer_gen(unsigned int size, struct UniformBuffer* dest) {
    if (!(dest->cache = malloc(size))) {
        return 0;
    }
    glGenBuffers(1, &dest->ubo);
    if (!dest->ubo) {
        free(dest->cache);
        return 0;
    }
    glBindBuffer(GL_UNIFORM_BUFFER, dest->ubo);
    glBufferData(GL_UNIFORM_BUFFER, size, NULL, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    dest->dStart = size;
    dest->dEnd = 0;
    dest->size = size;
    return 1;
}

void uniform_buffer_del(struct UniformBuffer* u) {
    free(u->cache);
    glDeleteBuffers(1, &u->ubo);
}

struct UniformBuffer* uniform_buffer_new(unsigned int size) {
    struct UniformBuffer* u;
    if ((u = malloc(sizeof(*u)))) {
        if (uniform_buffer_gen(size, u)) {
            return u;
        }
        free(u);
    }
    return NULL;
}

void uniform_buffer_free(struct UniformBuffer* u) {
    if (u) {
        uniform_buffer_del(u);
        free(u);
    }
}

void uniform_buffer_invalidate(struct UniformBuffer* u, unsigned int offset, unsigned int size) {
    if (u->dStart > offset) {
        u->dStart = offset;
    }
    if (u->dEnd < offset + size) {
        u->dEnd = offset + size;
    }
}

void uniform_buffer_update(struct UniformBuffer* u, unsigned int offset, unsigned int size, const void* data) {
    uniform_buffer_invalidate(u, offset, size);
    memcpy(((char*)u->cache) + offset, data, size);
}

void uniform_buffer_send(struct UniformBuffer* u) {
    if (u->dStart >= u->dEnd) return;
    glBindBuffer(GL_UNIFORM_BUFFER, u->ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, u->dStart, u->dEnd - u->dStart, ((char*)u->cache) + u->dStart);
    u->dStart = u->size;
    u->dEnd = 0;
}

void uniform_buffer_bind(struct UniformBuffer* u, GLuint slot) {
    glBindBufferRange(GL_UNIFORM_BUFFER, slot, u->ubo, 0, u->size);
}
