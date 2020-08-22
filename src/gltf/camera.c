#include "gltf.h"

static int parse_perspective(struct Camera* cam, json_t* perspective) {
    json_t *tmp;
    float yfov = 80. / 360. * 2. * M_PI, zfar = 1000., znear = 0.001;

    if (!(tmp = json_object_get(perspective, "yfov")) || !json_is_number(tmp)) return 0;
    yfov = json_number_value(tmp);
    if (!(tmp = json_object_get(perspective, "zfar")) || !json_is_number(tmp)) return 0;
    zfar = json_number_value(tmp);
    if (!(tmp = json_object_get(perspective, "znear")) || !json_is_number(tmp)) return 0;
    znear = json_number_value(tmp);

    // yfov = 80. / 360. * 2. * M_PI;
    camera_projection(1., yfov, znear, zfar, cam->projection);
    return 1;
}

int gltf_parse_cameras(struct GltfContext* context, json_t* jroot) {
    json_t* cur;
    unsigned int numCameras, i;

    if (!(cur = json_object_get(jroot, "cameras"))) {
        return 1;
    }
    if (!(numCameras = json_array_size(cur))) {
        return 0;
    }
    if (!(context->cameras = malloc(numCameras * sizeof(*context->cameras)))) {
        fprintf(stderr, "Error: gltf: camera: can't allocate memory\n");
        return 0;
    }
    context->numCameras = numCameras;
    for (i = 0; i < numCameras; i++) {
        json_t *cam, *persp;
        if (!(context->cameras[i] = malloc(sizeof(struct Camera)))) {
            fprintf(stderr, "Error: camera: can't allocate memory\n");
            return 0;
        }
        load_id4(context->cameras[i]->view);
        cam = json_array_get(cur, i);
        if (!(persp = json_object_get(cam, "perspective"))) {
            fprintf(stderr, "Error: gltf: camera: missing perspective\n");
            return 0;
        }
        if (!parse_perspective(context->cameras[i], persp)) {
            fprintf(stderr, "Error: gltf: camera: perspective\n");
            return 0;
        }
    }
    return 1;
}
