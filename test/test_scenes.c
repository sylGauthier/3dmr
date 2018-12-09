#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <unistd.h>

#include <game/asset_manager.h>
#include <game/material/phong_color.h>
#include <game/material/phong_texture.h>
#include <game/render/camera.h>
#include <game/render/globject.h>
#include <game/render/texture.h>
#include <game/render/viewer.h>
#include <game/scene/scene.h>

#include "test/util/scenes_basic.h"
#include "test/util/color.h"
#include "test/util/light.h"
#include "test/util/materials.h"
#include "test/util/checkerboard.h"
#include "test/util/callbacks.h"
#include "test/util/main.h"


int run(struct Viewer* viewer, struct Scene* scene, int argc, char** argv) {
    struct PhongTextureMaterial *mat_checkerboard;
    struct PhongColorMaterial *mat_solidcolor;

    viewer->key_callback = key_callback;
    viewer->close_callback = close_callback;
    running = 1;

    mat_checkerboard = phong_texture_material_new(default_checkerboard(), &phongDefaultMat);
    mat_solidcolor = phong_color_material_new(0.0, 1.0, 0.0, &phongDefaultMat);

    scene->lights.numPointLights = 1;
    test_init_point_light(&scene->lights.point[0]);

    spheres_and_boxes((struct Material*)mat_solidcolor, (struct Material*)mat_checkerboard, &scene->root);
    node_rotate(&scene->root, (void *)VEC3_AXIS_X, M_PI / 2.0);

    viewer->callbackData = &scene->root;

    return EXIT_SUCCESS;
}
