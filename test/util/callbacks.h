#ifndef TEST_UTIL_CALLBACKS_H
#define TEST_UTIL_CALLBACKS_H

extern int running;

void cursor_rotate_object(struct Viewer* viewer, double xpos, double ypos, double dx, double dy, int buttonLeft, int buttonMiddle, int buttonRight, void* data);
void cursor_rotate_camera(struct Viewer* viewer, double xpos, double ypos, double dx, double dy, int buttonLeft, int buttonMiddle, int buttonRight, void* data);
void wheel_callback(struct Viewer* viewer, double xoffset, double yoffset, void* userData);
void key_callback(struct Viewer* viewer, int key, int scancode, int action, int mods, void* userData);
void close_callback(struct Viewer* viewer, void* userData);

#endif
