#include <stdlib.h>
#include <string.h>

#include "opengex_common.h"

int ogex_parse_camera_object(struct OgexContext* context, struct ODDLStructure* cur) {
    unsigned int i;
    struct Camera* cam;
    float fov = 80.0 / 360.0 * (2.0 * M_PI), zNear = 0.001, zFar = 1000.0;

    /* If the camera is already in context, we skip and return success */
    if (ogex_get_shared_object(context, cur)) return 1;

    if (!(cam = malloc(sizeof(struct Camera)))) {
        fprintf(stderr, "Error: CameraObject: can't allocate memory for camera\n");
        return 0;
    }
    if (!(ogex_add_shared_object(context, cur, cam, 0))) {
        free(cam);
        fprintf(stderr, "Error: CameraObject: can't allocate memory for camera array\n");
        return 0;
    }

    for (i = 0; i < cur->nbStructures; i++) {
        char* attrib;
        float val;

        if (ogex_get_identifier(cur->structures[i]) == OGEX_PARAM) {
            if (!ogex_parse_param(cur->structures[i], &attrib, &val)) {
                return 0;
            }
            if (!strcmp(attrib, "fov") || !strcmp(attrib, "fovx")) {
                fov = val;
            } else if (!strcmp(attrib, "near")) {
                zNear = val;
            } else if (!strcmp(attrib, "far")) {
                zFar = val;
            }
        }
    }

    /* TODO: fix zFar, for some reason it's way too small when using actual values from ogex file. It's not a problem from the importer.
     * Temporarily we add a hack by multiplying it by 10. */
    camera_projection(1.0, fov * context->angle, zNear * context->scale, 10.0 * zFar * context->scale, cam->projection);
    return 1;
}
