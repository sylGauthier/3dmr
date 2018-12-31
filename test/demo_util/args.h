#include <game/render/globject.h>
#include <game/scene/node.h>
#include <game/scene/scene.h>

#ifndef DEMO_UTIL_ARGS_H
#define DEMO_UTIL_ARGS_H

struct Config {
    struct Scene scene;
    struct GLObject* skybox;
    struct Node** objects;
    unsigned int numObjects;
    const char* screenshot;
    const char* title;
    unsigned int width, height;
    int timeout;
};

void usage_viewer(void);

void config_init(struct Config* config);
void config_free(struct Config* config);
void config_object_free(struct Config* config, unsigned int i);

int parse_args(int* argc, char*** argv, struct Config* config, void (*usage)(const char*), int warnNoArgs);
int parse_args_objects(int* argc, char*** argv, struct Config* config);

int parse_float(const char* s, char** e, float* out);
int parse_vec2(const char* s, char** e, Vec2 out);
int parse_vec3(const char* s, char** e, Vec3 out);
int parse_vec4(const char* s, char** e, Vec4 out);

#endif
