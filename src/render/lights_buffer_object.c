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


GLuint lights_buffer_object(void) {
    GLuint ubo;

    glGenBuffers(1, &ubo);
    if (!ubo) return 0;
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferData(GL_UNIFORM_BUFFER, SIZEOF_LIGHTS_UBO, NULL, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBufferRange(GL_UNIFORM_BUFFER, LIGHTS_UBO_BINDING, ubo, 0, SIZEOF_LIGHTS_UBO);
    return ubo;
}

void lights_buffer_object_zero_init(GLuint ubo) {
    struct Lights lights = {0};
    lights_buffer_object_update(&lights, ubo);
}

void lights_buffer_object_update_ambient(const struct AmbientLight* l, GLuint ubo) {
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, OFFSET_AMBIENT, sizeof(Vec3), l->color);
}

void lights_buffer_object_update_dlight(const struct DirectionalLight* l, unsigned int i, GLuint ubo) {
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, OFFSET_DLIGHT_DIRECTION(i), sizeof(Vec3), l->direction);
    glBufferSubData(GL_UNIFORM_BUFFER, OFFSET_DLIGHT_COLOR(i), sizeof(Vec3), l->color);
}

void lights_buffer_object_update_ndlight(unsigned int n, GLuint ubo) {
    uint32_t gn = n;
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, OFFSET_NUM_DLIGHTS, sizeof(gn), &gn);
}

void lights_buffer_object_update_dlights(const struct Lights* l, GLuint ubo) {
    uint32_t i, n = l->numDirectionalLights;
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    for (i = 0; i < n; i++) {
        glBufferSubData(GL_UNIFORM_BUFFER, OFFSET_DLIGHT_DIRECTION(i), sizeof(Vec3), l->directional[i].direction);
        glBufferSubData(GL_UNIFORM_BUFFER, OFFSET_DLIGHT_COLOR(i), sizeof(Vec3), l->directional[i].color);
    }
    glBufferSubData(GL_UNIFORM_BUFFER, OFFSET_NUM_DLIGHTS, sizeof(n), &n);
}

void lights_buffer_object_update_plight(const struct PointLight* l, unsigned int i, GLuint ubo) {
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, OFFSET_PLIGHT_POSITION(i), sizeof(Vec3), l->position);
    glBufferSubData(GL_UNIFORM_BUFFER, OFFSET_PLIGHT_COLOR(i), sizeof(Vec3), l->color);
    glBufferSubData(GL_UNIFORM_BUFFER, OFFSET_PLIGHT_RADIUS(i), sizeof(float), &l->radius);
}

void lights_buffer_object_update_nplight(unsigned int n, GLuint ubo) {
    uint32_t gn = n;
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, OFFSET_NUM_PLIGHTS, sizeof(gn), &gn);
}

void lights_buffer_object_update_plights(const struct Lights* l, GLuint ubo) {
    uint32_t i, n = l->numPointLights;
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    for (i = 0; i < n; i++) {
        glBufferSubData(GL_UNIFORM_BUFFER, OFFSET_PLIGHT_POSITION(i), sizeof(Vec3), l->point[i].position);
        glBufferSubData(GL_UNIFORM_BUFFER, OFFSET_PLIGHT_COLOR(i), sizeof(Vec3), l->point[i].color);
        glBufferSubData(GL_UNIFORM_BUFFER, OFFSET_PLIGHT_RADIUS(i), sizeof(float), &l->point[i].radius);
    }
    glBufferSubData(GL_UNIFORM_BUFFER, OFFSET_NUM_PLIGHTS, sizeof(n), &n);
}

void lights_buffer_object_update(const struct Lights* l, GLuint ubo) {
    lights_buffer_object_update_ambient(&l->ambientLight, ubo);
    lights_buffer_object_update_dlights(l, ubo);
    lights_buffer_object_update_plights(l, ubo);
}
