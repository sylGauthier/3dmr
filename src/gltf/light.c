#include <string.h>

#include "gltf.h"

static int parse_directional_light(struct GltfLight* light, json_t* jlight) {
    json_t* tmp;

    light->type = GLTF_DIRECTIONAL;
    light->light.dir.color[0] = 1;
    light->light.dir.color[1] = 1;
    light->light.dir.color[2] = 1;

    if ((tmp = json_object_get(jlight, "color")) && json_array_size(tmp) >= 3) {
        light->light.dir.color[0] = json_number_value(json_array_get(tmp, 0));
        light->light.dir.color[1] = json_number_value(json_array_get(tmp, 1));
        light->light.dir.color[2] = json_number_value(json_array_get(tmp, 2));
    }
    return 1;
}

static int parse_spot_light(struct GltfLight* light, json_t* jlight) {
    json_t* tmp;

    light->type = GLTF_SPOT;
    light->light.spot.color[0] = 1;
    light->light.spot.color[1] = 0;
    light->light.spot.color[2] = 0;
    light->light.spot.intensity = 500;
    light->light.spot.outerAngle = M_PI / 4.;
    light->light.spot.innerAngle = 0.;

    if ((tmp = json_object_get(jlight, "color")) && json_array_size(tmp) >= 3) {
        light->light.spot.color[0] = json_number_value(json_array_get(tmp, 0));
        light->light.spot.color[1] = json_number_value(json_array_get(tmp, 1));
        light->light.spot.color[2] = json_number_value(json_array_get(tmp, 2));
    }
    if ((tmp = json_object_get(jlight, "intensity"))) {
        light->light.spot.intensity = json_number_value(tmp);
    }
    if ((tmp = json_object_get(jlight, "spot"))) {
        if ((tmp = json_object_get(tmp, "innerConeAngle"))) {
            light->light.spot.innerAngle = json_number_value(tmp);
        }
        if ((tmp = json_object_get(tmp, "outerConeAngle"))) {
            light->light.spot.outerAngle = json_number_value(tmp);
        }
    }
    return 1;
}

static int parse_point_light(struct GltfLight* light, json_t* jlight) {
    json_t* tmp;

    light->type = GLTF_POINT;
    light->light.point.color[0] = 1;
    light->light.point.color[1] = 1;
    light->light.point.color[2] = 1;
    light->light.point.radius = 1;

    if ((tmp = json_object_get(jlight, "color")) && json_array_size(tmp) >= 3) {
        light->light.point.color[0] = json_number_value(json_array_get(tmp, 0));
        light->light.point.color[1] = json_number_value(json_array_get(tmp, 1));
        light->light.point.color[2] = json_number_value(json_array_get(tmp, 2));
    }
    if ((tmp = json_object_get(jlight, "intensity"))) {
        light->light.point.radius = json_number_value(tmp) / 100.;
    }
    return 1;
}

static int parse_light(struct GltfLight* light, json_t* jlight) {
    json_t* tmp;
    const char* type;

    if (!(tmp = json_object_get(jlight, "type")) || !(type = json_string_value(tmp))) {
        fprintf(stderr, "Error: gltf: light: missing type\n");
        return 0;
    }
    if (!strcmp(type, "directional")) {
        return parse_directional_light(light, jlight);
    } else if (!strcmp(type, "point")) {
        return parse_point_light(light, jlight);
    } else if (!strcmp(type, "spot")) {
        return parse_spot_light(light, jlight);
    }
    fprintf(stderr, "Error: gltf: light: unknown light type\n");
    return 0;
}

int gltf_parse_lights(struct GltfContext* context, json_t* jroot) {
    json_t *tmp, *curLight;
    unsigned int idx;

    if (!(tmp = json_object_get(jroot, "extensions"))) return 1;
    if (!(tmp = json_object_get(tmp, "KHR_lights_punctual"))) return 1;
    if (!(tmp = json_object_get(tmp, "lights"))) return 1;
    if (!json_array_size(tmp)) return 1;

    if (!(context->lights = malloc(json_array_size(tmp) * sizeof(*context->lights)))) {
        fprintf(stderr, "Error: gltf: lights: could not allocate light array\n");
        return 0;
    }
    context->numLights = json_array_size(tmp);

    json_array_foreach(tmp, idx, curLight) {
        if (!parse_light(&context->lights[idx], curLight)) return 0;
    }

    return 1;
}

