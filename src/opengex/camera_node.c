#include <stdlib.h>
#include <string.h>

#include "opengex_common.h"

static struct Camera* parse_object_ref(struct OgexContext* context, struct ODDLStructure* cur) {
    struct ODDLStructure* camPtr;

    if (!ogex_parse_ref(cur, &camPtr)) {
        return NULL;
    }
    if (!ogex_parse_camera_object(context, camPtr)) {
        fprintf(stderr, "Warning: ObjectRef: parse_camera_object failed\n");
        return NULL;
    }
    return ogex_get_shared_object(context, camPtr);
}

int ogex_parse_camera_node(struct OgexContext* context, struct Node* node, struct ODDLStructure* cur) {
    unsigned int i;
    struct Camera* camTemplate = NULL;
    struct Camera* newCam;
    Vec3 axis = {1, 0, 0};

    if (!(cur->structures)) return 0;
    if (!(newCam = malloc(sizeof(*newCam)))) {
        fprintf(stderr, "Error: CameraNode: can't allocate memory for new camera\n");
        return 0;
    }

    for (i = 0; i < cur->nbStructures; i++) {
        struct ODDLStructure* tmp = cur->structures[i];

        switch (ogex_get_identifier(tmp)) {
            case OGEX_OBJECT_REF:
                if (!(camTemplate = parse_object_ref(context, tmp))) {
                    fprintf(stderr, "Error: CameraNode: invalid reference to camera object\n");
                    free(newCam);
                    return 0;
                }
                break;
            default:
                break;
        }
    }
    memcpy(newCam, camTemplate, sizeof(struct Camera));
    if (context->up == AXIS_Z) {
        node_slew(node, axis, -M_PI / 2.0);
    }
    node_set_camera(node, newCam);
    if (context->metadata) {
        struct Node** tmp;
        if (!(tmp = realloc(context->metadata->cameraNodes, (context->metadata->nbCameraNodes + 1) * sizeof(struct Node*)))) {
            fprintf(stderr, "Error: CameraNode: can't allocate memory for metadata\n");
            free(newCam);
            return 0;
        }
        context->metadata->cameraNodes = tmp;
        context->metadata->cameraNodes[context->metadata->nbCameraNodes++] = node;
    }
    if (context->shared) {
        if (!import_add_shared_item(&context->shared->cams, &context->shared->nbCams, newCam)) {
            free(newCam);
            return 0;
        }
    }
    return 1;
}
