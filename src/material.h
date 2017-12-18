#include "linear_algebra.h"

#ifndef MATERIAL_H
#define MATERIAL_H

struct Material
{
    Vec3 ambient;
    Vec3 diffuse;
    Vec3 specular;
    float shininess;
};

#endif
