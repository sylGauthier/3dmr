#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "context.h"
#include "object_ref.h"
#include "light_object.h"
#include "light_node.h"

static struct OgexLight* parse_light_object_ref(const struct OgexContext* context, const struct ODDLStructure* cur) {
    if (!(cur = ogex_parse_object_ref(cur))) return NULL;
    return ogex_context_find_object(&context->lights, cur);
}

int ogex_parse_light_node(const struct OgexContext* context, struct Node* node, const struct ODDLStructure* cur) {
    struct OgexLight* light = NULL;
    unsigned int i;

    for (i = 0; i < cur->nbStructures; i++) {
        struct ODDLStructure* tmp = cur->structures[i];

        if (!tmp->identifier) {
            continue;
        } else if (!strcmp(tmp->identifier, "ObjectRef")) {
            if (!(light = parse_light_object_ref(context, tmp))) {
                fprintf(stderr, "Error: LightNode: invalid reference to light object\n");
                return 0;
            }
        }
    }
    if (!light) {
        fprintf(stderr, "Error: LightNode: missing reference to light object\n");
        return 0;
    }
    switch (light->type) {
        case OGEX_LIGHT_INFINITE:
            {
                struct DirectionalLight* newLight;
                if (!(newLight = malloc(sizeof(*newLight)))) {
                    fprintf(stderr, "Error: LightObject: cannot allocate memory for new light\n");
                    return 0;
                }
                memcpy(newLight->color, light->color, sizeof(Vec3));
                node_set_dlight(node, newLight);
            }
            break;
        case OGEX_LIGHT_POINT:
            {
                struct PointLight* newLight;
                if (!(newLight = malloc(sizeof(*newLight)))) {
                    fprintf(stderr, "Error: LightObject: cannot allocate memory for new light\n");
                    return 0;
                }
                switch (light->atten.type) {
                    case ATTEN_SMOOTH:
                    case ATTEN_LINEAR:
                    case ATTEN_INVERSE:
                        fprintf(stderr, "Warning: LightObject: unsupported attenuation type (only inverse square is supported)\n");
                    case ATTEN_INVERSE_SQUARE:
                        break;
                    default:
                        fprintf(stderr, "Error: LightObject: unknown attenuation type\n");
                        free(newLight);
                        return 0;
                }
                memcpy(newLight->color, light->color, sizeof(Vec3));
                newLight->radius = light->atten.scale;
                node_set_plight(node, newLight);
            }
            break;
        case OGEX_LIGHT_SPOT:
            fprintf(stderr, "Error: LightObject: unsupported light object: spot\n");
            return 0;
        default:
            fprintf(stderr, "Error: LightNode: unsupported light type\n");
            return 0;
    }
    if (context->metadata && !import_add_metadata_lightnode(context->metadata, node)) return 0;
    return 1;
}
