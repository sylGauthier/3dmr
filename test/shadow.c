#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <3dmr/shaders.h>
#include <3dmr/scene/scene.h>

#include <3dmr/mesh/box.h>
#include <3dmr/mesh/quad.h>

#include <3dmr/render/lights_buffer_object.h>
#include <3dmr/render/camera_buffer_object.h>

#include <3dmr/material/pbr.h>
#include <3dmr/material/phong.h>
#include <3dmr/material/solid.h>

#define TDMR_SHADERS_PATH TDMR_SHADERS_PATH_SRC

struct PBRMaterialParams pbrp;
struct PhongMaterialParams phongp, phongp2;
struct SolidMaterialParams solidp, depthmapp;

int setup_depthmap_display(struct Node* disp, struct Geometry* g) {
    struct Mesh quadm;
    Vec3 t = {0.5, -0.5, 0};

    solid_material_params_init(&depthmapp);
    material_param_set_vec3_texture(&depthmapp.color, 0);
    depthmapp.floatTexture = 1;
    depthmapp.alpha.mode = ALPHA_DISABLED;

    make_quad(&quadm, 1, 1);
    g->vertexArray = vertex_array_new(&quadm);
    g->material = solid_overlay_material_new(quadm.flags, &depthmapp);
    node_init(disp);
    node_set_geometry(disp, g);
    node_translate(disp, t);

    return 1;
}

int setup_cubes(struct Node cubes[4], struct Geometry* g) {
    unsigned int i;
    struct Mesh cubem;
    int ok = 1;

    solid_material_params_init(&solidp);
    material_param_set_vec3_elems(&solidp.color, 1, 1, 1);
    solidp.alpha.mode = ALPHA_DISABLED;

    phong_material_params_init(&phongp);
    material_param_set_vec3_elems(&phongp.ambient, 0.6, 0.6, 0.6);
    material_param_set_vec3_elems(&phongp.diffuse, 0.6, 0.6, 0.6);
    material_param_set_vec3_elems(&phongp.specular, 1, 1, 1);
    material_param_set_float_constant(&phongp.shininess, 5);
    phongp.alpha.mode = ALPHA_DISABLED;

    if (       !make_box(&cubem, 1, 1, 1)
            || !(g->vertexArray = vertex_array_new(&cubem))
            || !(g->material = phong_material_new(cubem.flags, &phongp))) {
        fprintf(stderr, "Error: setup_cube_geometry failed\n");
        ok = 0;
    }

    mesh_free(&cubem);

    if (ok) {
        for (i = 0; i < 4; i++) {
            node_init(&cubes[i]);
            node_set_geometry(&cubes[i], g);
        }
    }
    return ok;
}

int setup_plane(struct Node* plane, struct Geometry* g) {
    struct Mesh planem;
    Vec3 scale = {5, 10, 10}, pos = {-2.5, 0, 0};

    make_quad(&planem, 1, 1);

    phong_material_params_init(&phongp2);
    material_param_set_vec3_elems(&phongp2.ambient, 0.3, 0.9, 0.3);
    material_param_set_vec3_elems(&phongp2.diffuse, 0.3, 0.9, 0.3);
    material_param_set_vec3_elems(&phongp2.specular, 1, 1, 1);
    material_param_set_float_constant(&phongp2.shininess, 0.3);
    phongp2.alpha.mode = ALPHA_DISABLED;

    g->vertexArray = vertex_array_new(&planem);
    g->material = phong_material_new(planem.flags, &phongp2);

    node_init(plane);
    node_set_geometry(plane, g);
    node_set_scale(plane, scale);
    node_set_pos(plane, pos);

    return 1;
}

int setup_pbr_plane(struct Node* plane, struct Geometry* g) {
    struct Mesh planem;
    Vec3 scale = {5, 10, 10}, pos = {2.5, 0, 0};

    make_quad(&planem, 1, 1);

    pbr_material_params_init(&pbrp);
    material_param_set_vec3_elems(&pbrp.albedo, 0.9, 0.3, 0.3);
    material_param_set_float_constant(&pbrp.metalness, 0);
    material_param_set_float_constant(&pbrp.roughness, 0.5);
    pbrp.alpha.mode = ALPHA_DISABLED;

    g->vertexArray = vertex_array_new(&planem);
    g->material = pbr_material_new(planem.flags, &pbrp);

    node_init(plane);
    node_set_geometry(plane, g);
    node_set_scale(plane, scale);
    node_set_pos(plane, pos);

    return 1;
}

int setup_lights(struct Lights* lights, struct Scene* scene) {
    struct DirectionalLight* dl;
    struct ShadowMap* map;
    Vec3 pos = {2.5, 5, 7.5}, dir = {-0.5, -1, -1.5}, lookAt = {0, 0, 0}, up = {0, 0, 1};

    if (!light_init(lights)) return 0;

    set3v(lights->ambientLight.color, 0.5, 0.5, 0.5);
    lights->numDirectionalLights++;
    dl = &lights->directional[0];
    memcpy(dl->direction, dir, sizeof(Vec3));
    set3v(dl->color, 0.3, 0.3, 0.3);

    if ((dl->shadow = light_shadowmap_new(lights, 1024, 1024)) < 0) return 0;
    map = &lights->shadowMaps[dl->shadow];
    camera_ortho_projection(10, 10, 1, 15, map->projection);
    camera_look_at(pos, lookAt, up, map->view);

    lights_buffer_object_update(&scene->lights, lights);

    return 1;
}

int setup_camera(struct Camera* cam) {
    Vec3 pos = {1, -7, 5.5}, lookAt = {0, 0, 0}, camUp = {0, 0, 1};

    camera_look_at(pos, lookAt, camUp, cam->view);
    camera_projection(1., 2 * M_PI * 60. / 360., 0.1, 1000., cam->projection);
    return 1;
}

void cube_update(struct Node cubes[4], float dt) {
    static float time = 0;
    int i;
    Vec3 t;

    for (i = 0; i < 4; i++) {
        set3v(t, -3. + 2. * i, 0, 1.5 + sin(2 * M_PI * (time + (float) i / 8.)));
        node_set_pos(&cubes[i], t);
    }
    time += dt;
}

int main() {
    struct Scene scene;
    struct Viewer* viewer = NULL;
    struct Node cubes[4];
    struct Node* shadowQueue[6];
    struct Node plane, pbrPlane, overlay;
    struct Camera cam;
    struct Geometry geom, planeGeom, dispGeom, pbrPlaneGeom;
    struct Lights lights;

#ifdef TDMR_SHADERS_PATH
    tdmrShaderRootPath = TDMR_SHADERS_PATH;
#endif

    node_init(&plane);

    if (!(viewer = viewer_new(768, 768, "depthmap"))) {
        fprintf(stderr, "Error: failed to start viewer\n");
        return 0;
    }

    if (       !setup_camera(&cam)
            || !scene_init(&scene, &cam)
            || !setup_cubes(cubes, &geom)
            || !setup_plane(&plane, &planeGeom)
            || !setup_pbr_plane(&pbrPlane, &pbrPlaneGeom)
            || !setup_lights(&lights, &scene)
            || !setup_depthmap_display(&overlay, &dispGeom)) {
        fprintf(stderr, "Error: setup failed\n");
    } else {
        unsigned int i;

        for (i = 0; i < 4; i++) {
            node_add_child(&scene.root, &cubes[i]);
            shadowQueue[i] = &cubes[i];
        }
        node_add_child(&scene.root, &plane);
        node_add_child(&scene.root, &pbrPlane);
        shadowQueue[4] = &plane;
        shadowQueue[5] = &pbrPlane;

        depthmapp.color.value.texture = lights.shadowMaps[0].tex;
        /* uncomment to display light's depthmap */
        /* node_add_child(&scene.root, &overlay); */

        scene_update_nodes(&scene, NULL, NULL);
        uniform_buffer_send(&scene.camera);
        uniform_buffer_send(&scene.lights);


        scene_update_render_queue(&scene, MAT_CONST_CAST(cam.view), MAT_CONST_CAST(cam.projection));
        while (1) {
            float dt;
            dt = viewer_next_frame(viewer);
            viewer_process_events(viewer);

            cube_update(cubes, dt);
            scene_update_nodes(&scene, NULL, NULL);
            light_shadowmap_render(&lights, 0, shadowQueue, 5);
            scene_render(&scene, &lights);
        }
    }

    if (viewer) viewer_free(viewer);
    return 0;
}

