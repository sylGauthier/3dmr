#include <stdlib.h>
#include <string.h>
#include <game/material/phong.h>

#include "context.h"
#include "object_ref.h"

static struct Geometry* parse_geometry_ref(const struct OgexContext* context, const struct ODDLStructure* cur) {
    if (!(cur = ogex_parse_object_ref(cur))) return NULL;
    return ogex_context_find_object(&context->geometries, cur);
}

static struct PhongMaterialParams* parse_material_ref(const struct OgexContext* context, const struct ODDLStructure* cur) {
    if (!(cur = ogex_parse_object_ref(cur))) return NULL;
    return ogex_context_find_object(&context->matParams, cur);
}

int ogex_parse_geometry_node(const struct OgexContext* context, struct Node* node, const struct ODDLStructure* cur) {
    unsigned int i;
    const struct Geometry* gtmp = NULL;
    struct Geometry* geom;
    struct PhongMaterialParams* matParams = NULL;

    for (i = 0; i < cur->nbStructures; i++) {
        struct ODDLStructure* tmp = cur->structures[i];

        if (!tmp->identifier) {
            continue;
        } else if (!strcmp(tmp->identifier, "ObjectRef")) {
            gtmp = parse_geometry_ref(context, tmp);
        } else if (!strcmp(tmp->identifier, "MaterialRef")) {
            matParams = parse_material_ref(context, tmp);
        }
    }
    if (!(gtmp && matParams)) {
        fprintf(stderr, "Error: GeometryNode: missing vertex array or material, aborting\n");
        return 0;
    }
    if (!(geom = malloc(sizeof(*geom)))) {
        fprintf(stderr, "Error: GeometryNode: failed to allocate geometry\n");
        return 0;
    }
    *geom = *gtmp;
    if (!(geom->material = phong_material_new(geom->vertexArray->flags, matParams))) {
        fprintf(stderr, "Error: GeometryNode: failed to create material\n");
        free(geom);
        return 0;
    }
    node_set_geometry(node, geom);
    return 1;
}
