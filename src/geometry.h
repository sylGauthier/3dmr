#include <GL/glew.h>
#include "linear_algebra.h"
#include "camera.h"
#include "globject.h"

#ifndef GEOMETRY_H
#define GEOMETRY_H

struct Geometry {
    struct GLObject glObject;
    Mat4 model;
    GLuint shader;
    GLuint mode;
    void (*prerender)(const struct Geometry*);
    void (*postrender)(const struct Geometry*);
};

void geometry_render(const struct Geometry* geometry, const struct Camera* camera);

#endif
