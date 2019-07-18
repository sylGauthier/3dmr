#include <GL/glew.h>
#include <game/render/camera.h>

#ifndef VIEWER_H
#define VIEWER_H

struct Viewer {
    struct Camera camera;
    unsigned int width, height;
    void (*cursor_callback)(struct Viewer*, double, double, double, double, int, int, int, void*);
    void (*wheel_callback)(struct Viewer*, double, double, void*);
    void (*key_callback)(struct Viewer*, int, int, int, int, void*);
    void (*resize_callback)(struct Viewer*, void*);
    void (*close_callback)(struct Viewer*, void*);
    void* callbackData;
};

struct Viewer* viewer_new(unsigned int width, unsigned int height, const char* title);
void viewer_free(struct Viewer* viewer);

unsigned int viewer_register_program_id(void);

GLuint viewer_get_program(struct Viewer* viewer, unsigned int id);
int viewer_set_program(struct Viewer* viewer, unsigned int id, GLuint program);
void viewer_set_title(struct Viewer* viewer, const char* title);
void viewer_make_current(struct Viewer* viewer);
struct Viewer* viewer_get_current(void);
void viewer_process_events(struct Viewer* viewer);
double viewer_next_frame(struct Viewer* viewer);
int viewer_screenshot(struct Viewer* viewer, const char* filename);

#endif
