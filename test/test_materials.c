#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <game/asset_manager.h>
#include <game/material/solid.h>
#include <game/material/phong.h>
#include <game/material/pbr.h>
#include <game/mesh/obj.h>
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
    PBR,
    NUM_MATERIALS
};
static const char* materialNames[NUM_MATERIALS] = {
    "solid color",
    "phong color",
    "solid texture",
    "phong texture",
    "phong texture+normal map",
    "PBR"
};
static struct Material* materials[NUM_MATERIALS];
static unsigned int curMaterial = PHONG_TEXTURE_NORMALMAP;
static struct GLObject obj = {0};
static float angle;
static float* direction;

void custom_key_callback(struct Viewer* viewer, int key, int scancode, int action, int mods, void* userData) {
    switch (key) {
        case GLFW_KEY_F2:
            if (action == GLFW_PRESS) {
                obj.material = materials[curMaterial = ((curMaterial + NUM_MATERIALS - 1) % NUM_MATERIALS)];
                printf("curMaterial=%s\n", materialNames[curMaterial]);
            }
            return;
        case GLFW_KEY_F3:
            if (action == GLFW_PRESS) {
                obj.material = materials[curMaterial = ((curMaterial + 1) % NUM_MATERIALS)];
                printf("curMaterial=%s\n", materialNames[curMaterial]);
            }
            return;
        case GLFW_KEY_F4:
            angle -= 0.1f;
            direction[0] = cos(angle);
            direction[2] = sin(angle);
            return;
        case GLFW_KEY_F5:
            angle += 0.1f;
            direction[0] = cos(angle);
            direction[2] = sin(angle);
            return;
    }
    key_callback(viewer, key, scancode, action, mods, userData);
}

int run(struct Viewer* viewer, struct Scene* scene, int argc, char** argv) {
    struct Mesh mesh = {0};
    char* path;
    int ret = 1;
    GLuint albedo, normalMap, metalness, roughness;

    if (!(path = asset_manager_find_file("meshes/cerberus.obj"))) {
        fprintf(stderr, "Error: failed to find the obj file\n");
    } else if (!make_obj(&mesh, path, 0, 1, 1)) {
        fprintf(stderr, "Error: failed to create mesh\n");
    } else if (!mesh_compute_tangents(&mesh)) {
        fprintf(stderr, "Error: failed to compute tangents/bitangents\n");
    } else {
        albedo = asset_manager_load_texture("textures/cerberus_albedo.png");
        normalMap = asset_manager_load_texture("textures/cerberus_normal.png");
        metalness = asset_manager_load_texture("textures/cerberus_metalness.png");
        roughness = asset_manager_load_texture("textures/cerberus_roughness.png");
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
        } else if (!(materials[PBR] = (struct Material*)pbr_material_new(albedo, normalMap, metalness, roughness))) {
            fprintf(stderr, "Error: failed to create PBR material\n");
        } else {
            obj.material = materials[curMaterial];
            scene->root.object = &obj;
            scene->lights.ambientLight.color[0] = 1;
            scene->lights.ambientLight.color[1] = 1;
            scene->lights.ambientLight.color[2] = 1;
            direction = scene->lights.directional[0].direction;
#if 1
            scene->lights.directional[0].color[0] = 1;
            scene->lights.directional[0].color[1] = 1;
            scene->lights.directional[0].color[2] = 1;
            scene->lights.directional[0].direction[0] = 1;
            scene->lights.directional[0].direction[1] = 0;
            scene->lights.directional[0].direction[2] = 0;
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
