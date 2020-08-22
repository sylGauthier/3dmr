#include <string.h>

#include "gltf.h"

int gltf_parse_light_node(struct GltfContext* context, struct Node* light, json_t* jnode) {
    json_t* ext;
    Vec3 axis = {1, 0, 0};
    Quaternion a, b;

    if ((ext = json_object_get(jnode, "KHR_lights_punctual"))) {
        json_t* idx;
        struct GltfLight* glight;
        unsigned int lightIdx;

        if ((idx = json_object_get(ext, "light"))
                && json_is_integer(idx)
                && json_integer_value(idx) >= 0
                && json_integer_value(idx) < context->numLights) {
            lightIdx = json_integer_value(idx);
            glight = &context->lights[lightIdx];
            switch (glight->type) {
                case GLTF_POINT:
                    light->type = NODE_PLIGHT;
                    if (!(light->data.plight = malloc(sizeof(*light->data.plight)))) return 0;
                    *light->data.plight = glight->light.point;
                    break;
                case GLTF_DIRECTIONAL:
                    light->type = NODE_DLIGHT;
                    if (!(light->data.dlight = malloc(sizeof(*light->data.dlight)))) return 0;
                    *light->data.dlight = glight->light.dir;
                    break;
                case GLTF_SPOT:
                    light->type = NODE_SLIGHT;
                    if (!(light->data.slight = malloc(sizeof(*light->data.slight)))) return 0;
                    *light->data.slight = glight->light.spot;
                    break;
            }
        } else {
            fprintf(stderr, "Warning: gltf: invalid light\n");
        }
    }
    memcpy(a, light->orientation, sizeof(a));
    quaternion_set_axis_angle(b, axis, M_PI / 2.);
    quaternion_mul(light->orientation, a, b);
    if (context->metadata) {
        return import_add_metadata_lightnode(context->metadata, light);
    }
    return 1;
}
