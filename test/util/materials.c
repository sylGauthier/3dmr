#include <game/material/phong.h>

const struct PhongMaterial phongDefaultMat = {
    {0.1, 0.1, 0.1},
    {0.6, 0.6, 0.6},
    {0.1, 0.1, 0.1},
    1.0
};

const struct PhongMaterial phongNoSpecularMat = {
    {0.1, 0.1, 0.1},
    {0.6, 0.6, 0.6},
    {0.0, 0.0, 0.0},
    1.0
};
