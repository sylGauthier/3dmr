#include <string.h>
#include <stdlib.h>

#include "opengex_common.h"

static void* parse_light_object_ref(struct OgexContext* context, struct ODDLStructure* cur, enum OgexLightType* type) {
    struct ODDLStructure* lightPtr;
    struct ODDLProperty* prop;

    if (!(ogex_parse_ref(cur, &lightPtr))) {
        return NULL;
    }
    if (!(prop = oddl_get_property(lightPtr, "type"))) {
        fprintf(stderr, "Error: LightObject: needs a \"type\" property\n");
        return 0;
    }
    if (!strcmp(prop->str, "infinite")) {
        *type = OGEX_LIGHT_INFINITE;
    } else if (!strcmp(prop->str, "point")) {
        *type = OGEX_LIGHT_POINT;
    } else if (!strcmp(prop->str, "spot")) {
        *type = OGEX_LIGHT_SPOT;
    } else {
        fprintf(stderr, "Error: LightObject: unknown light type: %s\n", prop->str);
        return 0;
    }
    if (!ogex_parse_light_object(context, lightPtr)) {
        fprintf(stderr, "Warning: ObjectRef: parse_light_object failed\n");
        return NULL;
    }
    return ogex_get_shared_object(context, lightPtr);
}

int ogex_parse_light_node(struct OgexContext* context, struct Node* newNode, struct ODDLStructure* cur) {
    enum OgexLightType type;
    void* lightObj;
    unsigned int i;

    for (i = 0; i < cur->nbStructures; i++) {
        struct ODDLStructure* tmp = cur->structures[i];

        switch (ogex_get_identifier(tmp)) {
            case OGEX_OBJECT_REF:
                if (!(lightObj = parse_light_object_ref(context, tmp, &type))) {
                    fprintf(stderr, "Error: LightNode: invalid reference to light object\n");
                    return 0;
                }
                switch (type) {
                    case OGEX_LIGHT_INFINITE:
                        node_set_dlight(newNode, lightObj);
                        break;
                    case OGEX_LIGHT_POINT:
                        node_set_plight(newNode, lightObj);
                        break;
                    default:
                        fprintf(stderr, "Error: LightNode: unsupported light type\n");
                        return 0;
                }
            default:
                break;
        }
    }
    if (context->metadata) {
        struct Node** tmp;
        if (!(tmp = realloc(context->metadata->lightNodes, (context->metadata->nbLightNodes + 1) * sizeof(struct Node*)))) {
            fprintf(stderr, "Error: LightNode: can't allocate memory for metadata\n");
            return 0;
        }
        context->metadata->lightNodes = tmp;
        context->metadata->lightNodes[context->metadata->nbLightNodes++] = newNode;
    }
    return 1;
}
