#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <game/asset_manager.h>
#include <game/material/solid_color.h>
#include <game/material/phong_color.h>
#include <game/material/solid_texture.h>
#include <game/material/phong_texture.h>
#include <game/material/phong_texture_normalmap.h>
#include <game/mesh/box.h>
#include <game/render/globject.h>
#include <game/render/viewer.h>
#include <game/scene/scene.h>

#include "test/util/materials.h"
#include "test/util/main.h"
#include "test/util/callbacks.h"

enum Materials {
    SOLID_COLOR,
    PHONG_COLOR,
    SOLID_TEXTURE,
    PHONG_TEXTURE,
    PHONG_TEXTURE_NORMALMAP,
    NUM_MATERIALS
};
static const char* materialNames[NUM_MATERIALS] = {
    "solid color",
    "phong color",
    "solid texture",
    "phong texture",
    "phong texture+normal map"
};
static struct Material* materials[NUM_MATERIALS];
static unsigned int curMaterial = PHONG_TEXTURE_NORMALMAP;
static struct GLObject obj = {0};

void custom_key_callback(struct Viewer* viewer, int key, int scancode, int action, int mods, void* userData) {
    if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_F2:
                obj.material = materials[curMaterial = ((curMaterial + NUM_MATERIALS - 1) % NUM_MATERIALS)];
                printf("curMaterial=%s\n", materialNames[curMaterial]);
                return;
            case GLFW_KEY_F3:
                obj.material = materials[curMaterial = ((curMaterial + 1) % NUM_MATERIALS)];
                printf("curMaterial=%s\n", materialNames[curMaterial]);
                return;
        }
    }
    key_callback(viewer, key, scancode, action, mods, userData);
}

int run(struct Viewer* viewer, struct Scene* scene, int argc, char** argv) {
    struct Mesh mesh = {0};
    int ret = 1;
    GLuint albedo, normalMap;

    if (!make_box(&mesh, 20, 20, 1)) {
        fprintf(stderr, "Error: failed to create mesh\n");
    } else if (!mesh_compute_tangents(&mesh)) {
        fprintf(stderr, "Error: failed to compute tangents/bitangents\n");
    } else {
        albedo = asset_manager_load_texture("textures/brickwall.png");
        normalMap = asset_manager_load_texture("textures/brickwall_normal.png");
        obj.vertexArray = vertex_array_new(&mesh);
        if (!(materials[SOLID_COLOR] = (struct Material*)solid_color_material_new(1, 0, 0))) {
            fprintf(stderr, "Error: failed to create phong texture material\n");
        } else if (!(materials[PHONG_COLOR] = (struct Material*)phong_color_material_new(1, 0, 0, &phongDefaultMat))) {
            fprintf(stderr, "Error: failed to create phong texture material\n");
        } else if (!(materials[SOLID_TEXTURE] = (struct Material*)solid_texture_material_new(albedo))) {
            fprintf(stderr, "Error: failed to create phong texture material\n");
        } else if (!(materials[PHONG_TEXTURE] = (struct Material*)phong_texture_material_new(albedo, &phongDefaultMat))) {
            fprintf(stderr, "Error: failed to create phong texture material\n");
        } else if (!(materials[PHONG_TEXTURE_NORMALMAP] = (struct Material*)phong_texture_normalmap_material_new(albedo, normalMap, &phongDefaultMat))) {
            fprintf(stderr, "Error: failed to create phong texture normalmap material\n");
        } else {
            obj.material = materials[curMaterial];
            scene->root.object = &obj;
            scene->lights.ambiantLight.color[0] = 1;
            scene->lights.ambiantLight.color[1] = 1;
            scene->lights.ambiantLight.color[2] = 1;
#if 0
            scene->lights.directional[0].color[0] = 1;
            scene->lights.directional[0].color[1] = 1;
            scene->lights.directional[0].color[2] = 1;
            scene->lights.directional[0].direction[0] = 0;
            scene->lights.directional[0].direction[1] = 0;
            scene->lights.directional[0].direction[2] = -1;
            scene->lights.numDirectionalLights = 1;
#else
            scene->lights.point[0].color[0] = 1;
            scene->lights.point[0].color[1] = 1;
            scene->lights.point[0].color[2] = 1;
            scene->lights.point[0].position[0] = 0;
            scene->lights.point[0].position[1] = 0;
            scene->lights.point[0].position[2] = 5;
            scene->lights.point[0].radius = 20;
            scene->lights.numPointLights = 1;
#endif
            viewer->key_callback = custom_key_callback;
            viewer->cursor_callback = cursor_rotate_object;
            viewer->wheel_callback = wheel_callback;
            viewer->callbackData = &scene->root;

            viewer_next_frame(viewer);
            scene_render(scene, &viewer->camera);

            ret = 0;
        }
    }

    mesh_free(&mesh);

    return ret;
}
