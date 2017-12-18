#ifndef VIEWER_H
#define VIEWER_H

typedef struct Viewer Viewer;

Viewer* viewer_new(unsigned int width, unsigned int height, const char* title);
void viewer_free(Viewer* viewer);

void viewer_set_callbacks(Viewer* viewer, void (*cursor_callback)(double, double, double, double, int, int, int, void*), void (*wheel_callback)(double, double, void*), void (*key_callback)(int, int, int, int, void*), void (*close_callback)(void*), void* userData);

void viewer_process_events(Viewer* viewer);
void viewer_next_frame(struct Viewer* viewer);

struct Camera* viewer_get_camera(Viewer* viewer);
int viewer_get_width(Viewer* viewer);
int viewer_get_height(Viewer* viewer);

#endif
