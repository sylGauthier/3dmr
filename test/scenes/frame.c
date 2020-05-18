#include <stdlib.h>
#include <3dmr/scene/frame.h>
#include <3dmr/scene/scene.h>

int frame_setup(struct Scene* scene) {
    struct Node* frame = make_frame();
    if (!frame) return 0;
    if (!scene_add(scene, frame)) {
        free_frame(frame);
        return 0;
    }
    return 1;
}

void frame_teardown(struct Scene* scene) {
    free_frame(scene->root.children[0]);
    free(scene->root.children);
    scene->root.children = NULL;
    scene->root.nbChildren = 0;
}
