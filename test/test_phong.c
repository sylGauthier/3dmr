#include <stdio.h>
#include <stdlib.h>

#include <game/asset_manager.h>
#include <game/material/phong.h>
#include <game/mesh/icosphere.h>
#include <game/render/globject.h>
#include <game/render/viewer.h>
#include <game/scene/scene.h>

#include "test/util/light.h"
#include "test/util/materials.h"
#include "test/util/main.h"

static struct GLObject obj = {0};

int run(struct Viewer* viewer, struct Scene* scene, int argc, char** argv) {
    struct Mesh mesh = {0};
    int ret = 1;

    if (!make_icosphere(&mesh, 2.0, 4)) {
        fprintf(stderr, "Error: failed to create mesh\n");
    } else {
        obj.vertexArray = vertex_array_new(&mesh);
        if (!(obj.material = (struct Material*)phong_color_material_new(1.0, 0.0, 1.0, &phongNoSpecularMat))) {
            fprintf(stderr, "Error: failed to create geometry\n");
        } else {
            scene->root.object = &obj;
            test_init_point_light(&scene->lights.point[0]);
            scene->lights.numPointLights = 1;

            viewer_next_frame(viewer);
            scene_render(scene, &viewer->camera);

            ret = 0;
        }
    }

    mesh_free(&mesh);

    return ret;
}
