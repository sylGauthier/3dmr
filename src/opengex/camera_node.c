#include <stdlib.h>
#include <string.h>
#include <game/scene/import.h>
#include "camera_node.h"
#include "object_ref.h"

int ogex_parse_camera_node(const struct OgexContext* context, struct Node* node, const struct ODDLStructure* cur) {
    const Mat4* projection = NULL;
    struct Camera* newCam;
    Vec3 axis = {1, 0, 0};
    unsigned int i;

    for (i = 0; i < cur->nbStructures; i++) {
        struct ODDLStructure* tmp = cur->structures[i];

        if (!tmp->identifier) {
            continue;
        } else if (!strcmp(tmp->identifier, "ObjectRef")) {
            const struct ODDLStructure* ref;
            if (projection) {
                fprintf(stderr, "Error: CameraNode: multiple references to camera object\n");
                return 0;
            } else if (!(ref = ogex_parse_object_ref(tmp)) || !(projection = ogex_context_find_object(&context->cameras, ref))) {
                fprintf(stderr, "Error: CameraNode: invalid reference to camera object\n");
                return 0;
            }
        }
    }
    if (!projection) {
        fprintf(stderr, "Error: CameraNode: no reference to camera object\n");
        return 0;
    }

    if (!(newCam = malloc(sizeof(*newCam)))) {
        fprintf(stderr, "Error: CameraNode: can't allocate memory for new camera\n");
        return 0;
    }

    memcpy(newCam->projection, *projection, sizeof(Mat4));
    if (context->up == AXIS_Z) {
        node_slew(node, axis, -M_PI / 2.0);
    }
    node_set_camera(node, newCam);
    if (context->metadata && !import_add_metadata_camnode(context->metadata, node)) return 0;
    return 1;
}
