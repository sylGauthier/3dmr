#include <stdlib.h>
#include <stdint.h>
#include <game/render/lights_buffer_object.h>

/* offsets and size of uniform Lights (as defined in direct.glsl) following the std140 layout rules */
#define OFFSET_AMBIENT 0
#define SIZEOF_AMBIENT sizeof(Vec4)
#define OFFSET_DLIGHTS (OFFSET_AMBIENT + SIZEOF_AMBIENT)
#define SIZEOF_DLIGHT (2 * sizeof(Vec4))
#define SIZEOF_DLIGHTS (MAX_DIRECTIONAL_LIGHTS * SIZEOF_DLIGHT)
#define OFFSET_DLIGHT(i) (OFFSET_DLIGHTS + (i) * SIZEOF_DLIGHT)
#define OFFSET_DLIGHT_DIRECTION(i) OFFSET_DLIGHT(i)
#define OFFSET_DLIGHT_COLOR(i) (OFFSET_DLIGHT_DIRECTION(i) + sizeof(Vec4))
#define OFFSET_PLIGHTS (OFFSET_DLIGHTS + SIZEOF_DLIGHTS)
#define SIZEOF_PLIGHT (2 * sizeof(Vec4))
#define SIZEOF_PLIGHTS (MAX_POINT_LIGHTS * SIZEOF_PLIGHT)
#define OFFSET_PLIGHT(i) (OFFSET_PLIGHTS + (i) * SIZEOF_PLIGHT)
#define OFFSET_PLIGHT_POSITION(i) OFFSET_PLIGHT(i)
#define OFFSET_PLIGHT_COLOR(i) (OFFSET_PLIGHT_POSITION(i) + sizeof(Vec4))
#define OFFSET_PLIGHT_RADIUS(i) (OFFSET_PLIGHT_COLOR(i) + sizeof(Vec3))
#define OFFSET_NUM_DLIGHTS (OFFSET_PLIGHTS + SIZEOF_PLIGHTS)
#define OFFSET_NUM_PLIGHTS (OFFSET_NUM_DLIGHTS + sizeof(uint32_t))
#define SIZEOF_LIGHTS_UBO (OFFSET_NUM_PLIGHTS + sizeof(uint32_t))


int lights_buffer_object_gen(struct UniformBuffer* dest) {
    if (!uniform_buffer_gen(SIZEOF_LIGHTS_UBO, dest)) return 0;
    glBindBufferRange(GL_UNIFORM_BUFFER, LIGHTS_UBO_BINDING, dest->ubo, 0, SIZEOF_LIGHTS_UBO);
    return 1;
}

struct UniformBuffer* lights_buffer_object_new(void) {
    struct UniformBuffer* u;
    if ((u = malloc(sizeof(*u)))) {
        if (lights_buffer_object_gen(u)) {
            return u;
        }
        free(u);
    }
    return NULL;
}

void lights_buffer_object_zero_init(struct UniformBuffer* u) {
    struct Lights lights = {0};
    lights_buffer_object_update(u, &lights);
}

void lights_buffer_object_update_ambient(struct UniformBuffer* u, const struct AmbientLight* l) {
    uniform_buffer_update(u, OFFSET_AMBIENT, sizeof(Vec3), l->color);
}

void lights_buffer_object_update_dlight(struct UniformBuffer* u, const struct DirectionalLight* l, unsigned int i) {
    uniform_buffer_update(u, OFFSET_DLIGHT_DIRECTION(i), sizeof(Vec3), l->direction);
    uniform_buffer_update(u, OFFSET_DLIGHT_COLOR(i), sizeof(Vec3), l->color);
}

void lights_buffer_object_update_ndlight(struct UniformBuffer* u, unsigned int n) {
    uint32_t gn = n;
    uniform_buffer_update(u, OFFSET_NUM_DLIGHTS, sizeof(gn), &gn);
}

void lights_buffer_object_update_dlights(struct UniformBuffer* u, const struct Lights* l) {
    unsigned int i, n = l->numDirectionalLights;
    for (i = 0; i < n; i++) {
        lights_buffer_object_update_dlight(u, l->directional + i, i);
    }
    lights_buffer_object_update_ndlight(u, n);
}

void lights_buffer_object_update_plight(struct UniformBuffer* u, const struct PointLight* l, unsigned int i) {
    uniform_buffer_update(u, OFFSET_PLIGHT_POSITION(i), sizeof(Vec3), l->position);
    uniform_buffer_update(u, OFFSET_PLIGHT_COLOR(i), sizeof(Vec3), l->color);
    uniform_buffer_update(u, OFFSET_PLIGHT_RADIUS(i), sizeof(float), &l->radius);
}

void lights_buffer_object_update_nplight(struct UniformBuffer* u, unsigned int n) {
    uint32_t gn = n;
    uniform_buffer_update(u, OFFSET_NUM_PLIGHTS, sizeof(gn), &gn);
}

void lights_buffer_object_update_plights(struct UniformBuffer* u, const struct Lights* l) {
    unsigned int i, n = l->numPointLights;
    for (i = 0; i < n; i++) {
        lights_buffer_object_update_plight(u, l->point + i, i);
    }
    lights_buffer_object_update_nplight(u, n);
}

void lights_buffer_object_update(struct UniformBuffer* u, const struct Lights* l) {
    lights_buffer_object_update_ambient(u, &l->ambientLight);
    lights_buffer_object_update_dlights(u, l);
    lights_buffer_object_update_plights(u, l);
}
