#include <string.h>
#include <stdlib.h>

#include "opengex_common.h"

static void import_vec3(struct OgexContext* context, Vec3 dest, const Vec3 src) {
    if (context->up == AXIS_Z) {
        dest[0] = src[0];
        dest[1] = src[2];
        dest[2] = -src[1];
    } else {
        memcpy(dest, src, sizeof(Vec3));
    }
}

static int parse_mesh(struct OgexContext* context, struct Mesh* mesh, struct ODDLStructure* cur) {
    float *positions = NULL, *normals = NULL, *texCoords = NULL, *tangents = NULL;
    uint32_t* indices = NULL;
    unsigned int i, nbPos = 0, nbNorm = 0, nbTex = 0, nbTan = 0, nbIndices = 0;

    if (!(cur->structures)) {
        return 0;
    }

    for (i = 0; i < cur->nbStructures; i++) {
        struct ODDLStructure* tmp = cur->structures[i];
        struct ODDLStructure* subTmp;
        struct ODDLProperty* prop;

        switch (ogex_get_identifier(tmp)) {
            case OGEX_VERTEX_ARRAY:
                if (!(prop = oddl_get_property(tmp, "attrib"))) {
                    fprintf(stderr, "Error: VertexArray: missing property: \"attrib\"\n");
                    return 0;
                }
                if (tmp->nbStructures != 1) {
                    fprintf(stderr, "Error: VertexArray: invalid number of substructures (expected 1)\n");
                    return 0;
                }
                subTmp = tmp->structures[0];
                if (!strcmp(prop->str, "position")) {
                    positions = subTmp->dataList;
                    nbPos = subTmp->nbVec;
                } else if (!strcmp(prop->str, "normal")) {
                    normals = subTmp->dataList;
                    nbNorm = subTmp->nbVec;
                } else if (!strcmp(prop->str, "texcoord")) {
                    texCoords = subTmp->dataList;
                    nbTex = subTmp->nbVec;
                } else if (!strcmp(prop->str, "tangent")) {
                    tangents = subTmp->dataList;
                    nbTan = subTmp->nbVec;
                } else {
                    fprintf(stderr, "Warning: VertexArray: unhandled attribute: %s\n", prop->str);
                }
                break;
            case OGEX_INDEX_ARRAY: /* TODO: Remove code duplication with skinning */
                if (tmp->nbStructures != 1) {
                    fprintf(stderr, "Error: VertexArray: invalid number of substructures in (expected 1)\n");
                    return 0;
                }
                subTmp = tmp->structures[0];
                if (!(subTmp->type == TYPE_UINT32 && subTmp->vecSize == 3)) {
                    fprintf(stderr, "Error: IndexArray: unsupported data format: %s[%d]\n", typeName[subTmp->type], subTmp->vecSize);
                    return 0;
                }
                indices = subTmp->dataList;
                nbIndices = 3 * subTmp->nbVec;
                break;
            case OGEX_SKIN:
                if (!(mesh->skin = malloc(sizeof(struct Skin)))) {
                    fprintf(stderr, "Error: Mesh: could not allocate memory for Skin\n");
                    return 0;
                }
                if (!(ogex_parse_skin(context, mesh->skin, tmp))) {
                    free(mesh->skin);
                    return 0;
                }
                break;
            default:
                break;
        }
    }
    if (!positions) {
        fprintf(stderr, "Error: Mesh: must have positions\n");
        return 0;
    }
    if ((nbNorm && nbNorm != nbPos) || (nbTex && nbTex != nbPos) || (nbTan && nbTan != nbPos)) {
        fprintf(stderr, "Error: Mesh: invalid number of normals or texcoords or tangents\n");
        return 0;
    }
    mesh->flags = (!!nbNorm * MESH_NORMALS) | (!!nbTex * MESH_TEXCOORDS) | (!!nbTan * MESH_TANGENTS) | (!!mesh->skin * MESH_SKIN);
    mesh->numVertices = nbPos;
    mesh->numIndices = nbIndices;
    if (mesh->skin && mesh->numVertices != mesh->skin->nbVertices) {
        fprintf(stderr, "Error: Mesh: Skin has inconsistent number of vertices (Mesh has %d, Skin has %d)\n", mesh->numVertices, mesh->skin->nbVertices);
        return 0;
    }
    if (!(mesh->vertices = malloc(MESH_SIZEOF_VERTICES(mesh)))) {
        fprintf(stderr, "Error: Mesh: could not allocate memory for vertices\n");
        return 0;
    }
    for (i = 0; i < mesh->numVertices; i++) {
        unsigned int offset = 0;
        unsigned int stride = MESH_FLOATS_PER_VERTEX(mesh);

        /* Positions */
        import_vec3(context, mesh->vertices + i * stride, positions + i * 3);
        offset += 3;

        /* Normals */
        if (MESH_HAS_NORMALS(mesh)) {
            import_vec3(context, mesh->vertices + i * stride + offset, normals + i * 3);
            offset += 3;
        }

        /* Tex coords */
        if (MESH_HAS_TEXCOORDS(mesh)) {
            mesh->vertices[i * stride + offset] = texCoords[i * 2];
            mesh->vertices[i * stride + offset + 1] = texCoords[i * 2 + 1];
            offset += 2;
        }

        /* Tangents */
        if (MESH_HAS_TANGENTS(mesh)) {
            import_vec3(context, mesh->vertices + i * stride + offset, tangents + i * 6);
            offset += 3;
            import_vec3(context, mesh->vertices + i * stride + offset, tangents + i * 6 + 3);
            offset += 3;
        }

        /* Skin */
        if (MESH_HAS_SKIN(mesh)) {
            mesh->vertices[i * stride + offset] = mesh->skin->indexArray[i * 2];
            mesh->vertices[i * stride + offset + 1] = mesh->skin->indexArray[i * 2 + 1];
            mesh->vertices[i * stride + offset + 2] = mesh->skin->weightArray[i * 2];
            mesh->vertices[i * stride + offset + 3] = mesh->skin->weightArray[i * 2 + 1];
        }
    }
    if (nbIndices) {
        if (!(mesh->indices = malloc(nbIndices * sizeof(*mesh->indices)))) {
            fprintf(stderr, "Error: Mesh: could not allocate memory for indices\n");
            free(mesh->vertices);
            return 0;
        }
        /* Loop copy instead of memcpy for ensuring correct type size conversion */
        for (i = 0; i < nbIndices; i++) {
            mesh->indices[i] = indices[i];
        }
    } else {
        mesh->indices = NULL;
    }
    return 1;
}

static int parse_morph(struct OgexContext* context, struct ODDLStructure* cur) {
    fprintf(stderr, "Warning: Morph: not implemented\n");
    return 1;
}

int ogex_parse_geometry_object(struct OgexContext* context, struct ODDLStructure* cur) {
    unsigned int i, nbMeshes = 0;
    struct Mesh mesh;
    struct VertexArray* tmpVA;

    mesh.vertices = NULL;
    mesh.indices = NULL;
    mesh.numVertices = 0;
    mesh.numIndices = 0;
    mesh.flags = 0;
    mesh.skin = NULL;

    /* Geometry object has already been parsed and loaded in the context */
    if (ogex_get_shared_object(context, cur)) return 1;

    if (!(cur->structures)) {
        return 0;
    }

    for (i = 0; i < cur->nbStructures; i++) {
        struct ODDLStructure* tmp = cur->structures[i];

        switch (ogex_get_identifier(tmp)) {
            case OGEX_MESH:
                if (nbMeshes) {
                    fprintf(stderr, "Warning: GeometryObject: multiple meshes is not supported\n");
                } else {
                    if (!(parse_mesh(context, &mesh, tmp))) return 0;
                }
                nbMeshes++;
                break;
            case OGEX_MORPH:
                if (!(parse_morph(context, tmp))) return 0;
                break;
            default:
                break;
        }
    }
    if (!nbMeshes) {
        fprintf(stderr, "Error: GeometryObject: no valid mesh parsed?\n");
        return 0;
    }
    tmpVA = vertex_array_new(&mesh);
    mesh_free(&mesh);
    if (tmpVA->skin) {
        if (!skin_gen(tmpVA->skin)) {
            return 0;
        }
    }
    if (!(ogex_add_shared_object(context, cur, tmpVA, 1))) {
        fprintf(stderr, "Error: GeometryObject: couldn't reallocate memory for opengex context\n");
        vertex_array_free(tmpVA);
        return 0;
    }
    if (context->shared) {
        if (!import_add_shared_item(&context->shared->va, &context->shared->nbVA, tmpVA)) {
            vertex_array_free(tmpVA);
            return 0;
        }
    }
    return 1;
}
