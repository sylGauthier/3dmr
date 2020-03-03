#include <string.h>
#include "axis.h"
#include "common.h"
#include "metric.h"

int ogex_parse_metric(struct OgexContext* context, const struct ODDLStructure* cur) {
    struct ODDLProperty* prop;
    struct ODDLStructure* tmp;

    if (!(prop = oddl_get_property(cur, "key"))) {
        fprintf(stderr, "Error: Metric: Metric needs a \"key\" property\n");
        return 0;
    }
    if (cur->nbStructures != 1) {
        fprintf(stderr, "Error: Metric: expected exactly 1 sub structure\n");
        return 0;
    }
    tmp = cur->structures[0];
    if (!strcmp(prop->str, "distance")) {
        if (!ogex_check_struct(tmp, "Metric", TYPE_FLOAT32, 1, 1)) return 0;
        context->scale = *((float*)tmp->dataList);
    } else if (!strcmp(prop->str, "angle")) {
        if (!ogex_check_struct(tmp, "Metric", TYPE_FLOAT32, 1, 1)) return 0;
        context->angle = *((float*)tmp->dataList);
    } else if (!strcmp(prop->str, "time")) {
        if (!ogex_check_struct(tmp, "Metric", TYPE_FLOAT32, 1, 1)) return 0;
        context->time = *((float*)tmp->dataList);
    } else if (!strcmp(prop->str, "up")) {
        if (!ogex_check_struct(tmp, "Metric", TYPE_STRING, 1, 1)) return 0;
        context->up = ogex_parse_axis(((char**)tmp->dataList)[0]);
        if (!(context->up == AXIS_Y || context->up == AXIS_Z)) {
            fprintf(stderr, "Error: Metric: invalid up axis\n");
            return 0;
        }
    } else if (!strcmp(prop->str, "forward")) {
        if (!ogex_check_struct(tmp, "Metric", TYPE_STRING, 1, 1)) return 0;
        if (!(context->forward = ogex_parse_axis(((char**)tmp->dataList)[0]))) {
            fprintf(stderr, "Error: Metric: invalid forward axis\n");
            return 0;
        }
    } else {
        fprintf(stderr, "Error: Metric: unknown key: %s\n", prop->str);
        return 0;
    }
    return 1;
}

int ogex_parse_metrics(struct OgexContext* context, const struct ODDLStructure* root) {
    unsigned int i;

    for (i = 0; i < root->nbStructures; i++) {
        struct ODDLStructure* cur = root->structures[i];

        if (cur->identifier && !strcmp(cur->identifier, "Metric")) {
            if (!(ogex_parse_metric(context, cur))) return 0;
        }
    }

    return 1;
}
