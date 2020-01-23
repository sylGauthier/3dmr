#include <stdlib.h>
#include <string.h>
#include <game/material/phong.h>
#include <game/render/vertex_shader.h>

#include "opengex_common.h"

static struct VertexArray* parse_object_ref(struct OgexContext* context, struct ODDLStructure* cur) {
    struct ODDLStructure* goPtr;

    if (!ogex_parse_ref(cur, &goPtr)) {
        return NULL;
    }
    if (!ogex_parse_geometry_object(context, goPtr)) {
        fprintf(stderr, "Error: parsing GeometryObject failed\n");
        return NULL;
    }
    return ogex_get_shared_object(context, goPtr);
}

static struct PhongMaterialParams* parse_material_ref(struct OgexContext* context, struct ODDLStructure* cur) {
    struct ODDLStructure* matPtr;

    if (!ogex_parse_ref(cur, &matPtr)) {
        return NULL;
    }
    if (!ogex_parse_material(context, matPtr)) {
        fprintf(stderr, "Error: parsing Material failed\n");
        return NULL;
    }
    return ogex_get_shared_object(context, matPtr);
}

int ogex_parse_geometry_node(struct OgexContext* context, struct Node* node, struct ODDLStructure* cur) {
    unsigned int i;
    struct Geometry* glObject;
    struct VertexArray* va = NULL;
    struct PhongMaterialParams* matParams = NULL;
    struct Material* mat = NULL;

    if (!(cur->structures)) return 0;
    if (!(glObject = malloc(sizeof(*glObject)))) {
        fprintf(stderr, "Error: ogex_parse_geometry_node: can't allocate memory for new glObject\n");
        return 0;
    }

    for (i = 0; i < cur->nbStructures; i++) {
        struct ODDLStructure* tmp = cur->structures[i];

        switch (ogex_get_identifier(tmp)) {
            case OGEX_OBJECT_REF:
                va = parse_object_ref(context, tmp);
                break;
            case OGEX_MATERIAL_REF:
                matParams = parse_material_ref(context, tmp);
                break;
            default:
                break;
        }
    }
    if (!(va && matParams)) {
        fprintf(stderr, "Error: GeometryNode: missing vertex array or material, aborting\n");
        return 0;
    }
    if (!(mat = phong_material_new(va->flags, matParams))) {
        fprintf(stderr, "Error: GeometryNode: failed to create material\n");
        return 0;
    }
    glObject->vertexArray = va;
    glObject->material = mat;
    glObject->vertParams = NULL;
    glObject->fragParams = matParams;
    if (va->flags & MESH_SKIN) {
        mat->vertex_load = vertex_standard_load_skinned;
        glObject->vertParams = *(struct Skin**)(va + 1);
    }
    if (context->shared) {
        if (!import_add_shared_item(&context->shared->mats, &context->shared->nbMat, mat)) {
            free(mat);
            return 0;
        }
    }
    node_set_geometry(node, glObject);
    return 1;
}
