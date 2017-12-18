#include "globject.h"
#include "camera.h"
#include "linear_algebra.h"

void draw_solid_color(const struct GLObject* glo, GLuint shader, const struct Camera* camera, Mat4 model, Vec3 color);
