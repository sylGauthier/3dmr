#include <GL/glew.h>
#include "linear_algebra.h"
#include "camera.h"
#include "globject.h"

#ifndef GEOMETRY_H
#define GEOMETRY_H

struct Geometry {
    struct GLObject glObject;
    GLuint shader;
    GLuint mode;
    void (*prerender)(const struct Geometry*, const struct Camera*);
    void (*postrender)(const struct Geometry*, const struct Camera*);
};

void geometry_render(const struct Geometry* geometry, const struct Camera* camera, Mat4 model);

#endif
