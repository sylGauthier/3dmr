#include <game/render/camera_buffer_object.h>
#include <game/render/lights_buffer_object.h>
#include <game/scene/scene.h>

int scene_init(struct Scene* scene, struct Camera* camera) {
    light_init(&scene->lights);
    node_init(&scene->root, NULL);
    scene->root.alwaysDraw = 1;
    if (!(scene->uboCamera = camera_buffer_object())) {
        return 0;
    }
    if (!(scene->uboLights = lights_buffer_object())) {
        glDeleteBuffers(1, &scene->uboCamera);
        return 0;
    }
    scene->camera = camera;
    camera_buffer_object_update(camera, scene->uboCamera);
    lights_buffer_object_update(&scene->lights, scene->uboLights);
    return 1;
}

void scene_free(struct Scene* scene) {
    graph_free(&scene->root);
    glDeleteBuffers(1, &scene->uboCamera);
    glDeleteBuffers(1, &scene->uboLights);
}

int scene_add(struct Scene* scene, struct Node* node) {
    return node_add_child(&scene->root, node);
}

void scene_render(struct Scene* scene) {
    render_graph(&scene->root, scene->camera, &scene->lights);
}

unsigned int scene_render_count(struct Scene* scene) {
    return render_graph_count(&scene->root, scene->camera, &scene->lights);
}
