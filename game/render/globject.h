#include <game/light/light.h>
#include <game/math/linear_algebra.h>
#include <game/render/camera.h>
#include <game/render/material.h>
#include <game/render/vertex_array.h>

#ifndef GLOBJECT_H
#define GLOBJECT_H

struct GLObject {
    struct VertexArray* vertexArray;
    struct Material* material;
};

void globject_render(const struct GLObject* glObject, Mat4 model, Mat3 inverseNormal);

#endif
