#include <stdlib.h>
#include <string.h>
#include "camera_object.h"
#include "param.h"

Mat4* ogex_parse_camera_object(const struct OgexContext* context, const struct ODDLStructure* cur) {
    unsigned int i;
    Mat4* cam;
    float fov = 80.0 / 360.0 * (2.0 * M_PI), zNear = 0.001, zFar = 1000.0;

    for (i = 0; i < cur->nbStructures; i++) {
        struct ODDLStructure* tmp = cur->structures[i];
        char* attrib;
        float val;

        if (tmp->identifier && !strcmp(tmp->identifier, "Param")) {
            if (!ogex_parse_param(tmp, &attrib, &val)) return 0;
            if (!strcmp(attrib, "fov") || !strcmp(attrib, "fovx")) {
                fov = val;
            } else if (!strcmp(attrib, "near")) {
                zNear = val;
            } else if (!strcmp(attrib, "far")) {
                zFar = val;
            }
        }
    }

    if (!(cam = malloc(sizeof(*cam)))) {
        fprintf(stderr, "Error: CameraObject: can't allocate memory for camera\n");
        return 0;
    }

    /* TODO: fix zFar, for some reason it's way too small when using actual values from ogex file. It's not a problem from the importer.
     * Temporarily we add a hack by multiplying it by 10. */
    camera_projection(1.0, fov * context->angle, zNear * context->scale, 10.0 * zFar * context->scale, *cam);
    return cam;
}
