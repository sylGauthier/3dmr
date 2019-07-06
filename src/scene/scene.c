#include <stdlib.h>
#include <game/render/camera_buffer_object.h>
#include <game/render/lights_buffer_object.h>
#include <game/scene/scene.h>

int scene_init(struct Scene* scene, struct Camera* camera) {
    light_init(&scene->lights);
    node_init(&scene->root);
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
    scene->renderQueue = NULL;
    scene->nRender = 0;
    scene->aRender = 0;
    return 1;
}

void scene_free(struct Scene* scene, void (*free_node)(struct Node*)) {
    nodes_free(&scene->root, free_node);
    glDeleteBuffers(1, &scene->uboCamera);
    glDeleteBuffers(1, &scene->uboLights);
    free(scene->renderQueue);
}

int scene_add(struct Scene* scene, struct Node* node) {
    return node_add_child(&scene->root, node);
}

int scene_update_nodes(struct Scene* scene) {
    struct Node *cur, *next;
    int down = 1;
    unsigned int changed = 0;

    for (cur = &scene->root; cur; cur = next) {
        if (down) {
            changed |= cur->changedFlags;
            if (node_update_matrices(cur)) {
                switch (cur->type) {
                    /* we can't handle geometries there: camera might not be updated yet so node_visible cannot be called yet */
                    case NODE_DLIGHT:
                        lights_buffer_object_update_dlight(cur->data.dlight, cur->data.dlight - scene->lights.directional, scene->uboLights);
                        break;
                    case NODE_PLIGHT:
                        lights_buffer_object_update_plight(cur->data.plight, cur->data.plight - scene->lights.point, scene->uboLights);
                        break;
                    case NODE_CAMERA:
                        if (cur->data.camera == scene->camera) {
                            camera_buffer_object_update_view(MAT_CONST_CAST(scene->camera->view), scene->uboCamera);
                            camera_buffer_object_update_position(scene->camera->position, scene->uboCamera);
                        }
                        break;
                    default:;
                }
            }
            if (cur->nbChildren) {
                next = cur->children[0];
            } else {
                down = 0;
                node_update_father_bounding_box(cur);
                next = cur->father;
            }
        } else {
            unsigned int i;
            for (i = 0; i < cur->nbChildren && !cur->children[i]->changedFlags; i++);
            if (i < cur->nbChildren) {
                next = cur->children[i];
                down = 1;
            } else {
                node_update_father_bounding_box(cur);
                next = cur->father;
            }
        }
    }
    return !!changed;
}

int scene_update_render_queue(struct Scene* scene) {
    struct Node *cur, *next, **tmp;
    unsigned int i;

    scene->nRender = 0;
    for (cur = &scene->root; cur; cur = next) {
        if (node_visible(cur, scene->camera)) {
            if (cur->type == NODE_GEOMETRY) {
                if (scene->nRender + 1 < scene->aRender) {
                    scene->renderQueue[(scene->nRender)++] = cur;
                } else if (scene->aRender >= ((unsigned int)-2)
                        || !(tmp = realloc(scene->renderQueue, (scene->aRender + 1) * sizeof(*scene->renderQueue)))) {
                    return 0;
                } else {
                    scene->renderQueue = tmp;
                    scene->renderQueue[(scene->nRender)++] = cur;
                    scene->aRender++;
                }
            }
            if (cur->nbChildren) {
                next = cur->children[0];
                continue;
            }
        }
        while ((next = cur->father)) {
            for (i = 0; i < next->nbChildren && next->children[i] != cur; i++);
            if (i < next->nbChildren - 1) {
                next = next->children[i + 1];
                break;
            }
            cur = next;
        }
    }
    return 1;
}

void scene_render(struct Scene* scene) {
    unsigned int i;
    for (i = 0; i < scene->nRender; i++) {
        globject_render(scene->renderQueue[i]->data.geometry, &scene->lights, scene->renderQueue[i]->model, scene->renderQueue[i]->inverseNormal);
    }
}
