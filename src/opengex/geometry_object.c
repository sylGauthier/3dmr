#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "context.h"
#include "geometry_object.h"
#include "skin.h"

static void import_vec3(const struct OgexContext* context, Vec3 dest, const Vec3 src) {
    if (context->up == AXIS_Z) {
        dest[0] = src[0];
        dest[1] = src[2];
        dest[2] = -src[1];
    } else {
        memcpy(dest, src, sizeof(Vec3));
    }
}

static int parse_mesh(struct OgexContext* context, struct Mesh* mesh, struct Skin** skin, const struct ODDLStructure* cur) {
    float *positions = NULL, *normals = NULL, *texCoords = NULL, *tangents = NULL, *boneWeights = NULL;
    uint32_t* indices = NULL;
    unsigned int* boneIndices = NULL;
    unsigned int i, numPos = 0, numNorm = 0, numTex = 0, numTan = 0, numIndices = 0, numVerticesSkin = 0;
    int ok = 1;

    mesh->flags = 0;
    for (i = 0; ok && i < cur->nbStructures; i++) {
        struct ODDLStructure* tmp = cur->structures[i];
        struct ODDLStructure* subTmp;
        struct ODDLProperty* prop;

        if (!tmp->identifier) {
            continue;
        } else if (!strcmp(tmp->identifier, "VertexArray")) {
            if (!(prop = oddl_get_property(tmp, "attrib"))) {
                fprintf(stderr, "Error: VertexArray: missing property: \"attrib\"\n");
                ok = 0;
            } else if (tmp->nbStructures != 1) {
                fprintf(stderr, "Error: VertexArray: invalid type/number of substructures (expected 1)\n");
                ok = 0;
            } else {
                subTmp = tmp->structures[0];
                if (!strcmp(prop->str, "position")) {
                    positions = subTmp->dataList;
                    numPos = subTmp->nbVec;
                    ok = ogex_check_struct(subTmp, "VertexArray", TYPE_FLOAT32, subTmp->nbVec, 3);
                } else if (!strcmp(prop->str, "normal")) {
                    normals = subTmp->dataList;
                    numNorm = subTmp->nbVec;
                    mesh->flags |= MESH_NORMALS;
                    ok = ogex_check_struct(subTmp, "VertexArray", TYPE_FLOAT32, subTmp->nbVec, 3);
                } else if (!strcmp(prop->str, "texcoord")) {
                    texCoords = subTmp->dataList;
                    numTex = subTmp->nbVec;
                    mesh->flags |= MESH_TEXCOORDS;
                    ok = ogex_check_struct(subTmp, "VertexArray", TYPE_FLOAT32, subTmp->nbVec, 2);
                } else if (!strcmp(prop->str, "tangent")) {
                    tangents = subTmp->dataList;
                    numTan = subTmp->nbVec;
                    mesh->flags |= MESH_TANGENTS;
                    ok = ogex_check_struct(subTmp, "VertexArray", TYPE_FLOAT32, subTmp->nbVec, 3);
                } else {
                    fprintf(stderr, "Warning: VertexArray: unhandled attribute: %s\n", prop->str);
                }
            }
        } else if (!strcmp(tmp->identifier, "IndexArray")) {
            if (tmp->nbStructures != 1) {
                fprintf(stderr, "Error: IndexArray: invalid number of substructures in (expected 1)\n");
                ok = 0;
            } else if (!((subTmp = tmp->structures[0])->type == TYPE_UINT32 && subTmp->vecSize == 3)) {
                fprintf(stderr, "Error: IndexArray: unsupported data format: %s[%d]\n", typeName[subTmp->type], subTmp->vecSize);
                ok = 0;
            } else {
                indices = subTmp->dataList;
                numIndices = 3 * subTmp->nbVec;
            }
        } else if (!strcmp(tmp->identifier, "Skin")) {
            if (mesh->flags & MESH_SKIN) {
                fprintf(stderr, "Error: Mesh: cannot have multiple Skin\n");
                ok = 0;
            } else if (!(ok = ogex_parse_skin(context, tmp, skin, &boneIndices, &boneWeights, &numVerticesSkin))) {
                boneIndices = NULL;
                boneWeights = NULL;
            }
            mesh->flags |= MESH_SKIN;
        }
    }
    if (!positions) {
        fprintf(stderr, "Error: Mesh: must have positions\n");
        ok = 0;
    }
    if ((numNorm && numNorm != numPos) || (numTex && numTex != numPos) || (numTan && numTan != numPos)) {
        fprintf(stderr, "Error: Mesh: invalid number of normals or texcoords or tangents\n");
        ok = 0;
    }
    mesh->numVertices = numPos;
    mesh->numIndices = numIndices;
    if (ok && (mesh->flags & MESH_SKIN) && mesh->numVertices != numVerticesSkin) {
        fprintf(stderr, "Error: Mesh: Skin has inconsistent number of vertices (Mesh has %d, Skin has %d)\n", mesh->numVertices, numVerticesSkin);
        ok = 0;
    }
    if (ok && !(mesh->vertices = malloc(MESH_SIZEOF_VERTICES(mesh)))) {
        fprintf(stderr, "Error: Mesh: could not allocate memory for vertices\n");
        ok = 0;
    }
    if (!ok) {
        free(boneIndices);
        free(boneWeights);
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
            mesh->vertices[i * stride + offset] = boneIndices[i * 2];
            mesh->vertices[i * stride + offset + 1] = boneIndices[i * 2 + 1];
            mesh->vertices[i * stride + offset + 2] = boneWeights[i * 2];
            mesh->vertices[i * stride + offset + 3] = boneWeights[i * 2 + 1];
        }
    }
    free(boneIndices);
    free(boneWeights);
    if (numIndices) {
        if (!(mesh->indices = malloc(numIndices * sizeof(*mesh->indices)))) {
            fprintf(stderr, "Error: Mesh: could not allocate memory for indices\n");
            free(mesh->vertices);
            return 0;
        }
        /* Loop copy instead of memcpy for ensuring correct type size conversion */
        for (i = 0; i < numIndices; i++) {
            mesh->indices[i] = indices[i];
        }
    } else {
        mesh->indices = NULL;
    }
    return 1;
}

static int parse_morph(const struct OgexContext* context, const struct ODDLStructure* cur) {
    fprintf(stderr, "Warning: Morph: not implemented\n");
    return 1;
}

struct Geometry* ogex_parse_geometry_object(struct OgexContext* context, const struct ODDLStructure* cur) {
    unsigned int i, nbMeshes = 0;
    struct Mesh mesh;
    struct Skin* skin = NULL;
    struct VertexArray* tmpVA = NULL;
    struct Geometry* geom;
    int ok = 1;

    for (i = 0; ok && i < cur->nbStructures; i++) {
        struct ODDLStructure* tmp = cur->structures[i];

        if (!tmp->identifier) {
            continue;
        } else if (!strcmp(tmp->identifier, "Mesh")) {
            if (nbMeshes) {
                fprintf(stderr, "Warning: GeometryObject: multiple meshes is not supported\n");
            } else {
                if (!(parse_mesh(context, &mesh, &skin, tmp))) {
                    ok = 0;
                } else {
                    nbMeshes++;
                }
            }
        } else if (!strcmp(tmp->identifier, "Morph")) {
            if (!(parse_morph(context, tmp))) ok = 0;
        }
    }
    if (!nbMeshes) {
        fprintf(stderr, "Error: GeometryObject: no valid mesh parsed?\n");
        ok = 0;
    } else if (!(tmpVA = vertex_array_new(&mesh))) {
        fprintf(stderr, "Error: GeometryObject: couldn't allocate memory for vertex array\n");
        ok = 0;
    }
    if (nbMeshes) mesh_free(&mesh);
    if (!ok) return NULL;
    if (!(geom = malloc(sizeof(*geom)))) {
        fprintf(stderr, "Error: GeometryObject: couldn't allocate memory for vertex array\n");
        vertex_array_free(tmpVA);
        return NULL;
    }
    geom->vertexArray = tmpVA;
    geom->material = NULL;
    geom->vertParams = skin;
    geom->fragParams = NULL;
    return geom;
}

void ogex_free_geometry(void* p) {
    struct Geometry* g = p;
    if (g) {
        vertex_array_free(g->vertexArray);
        free(g);
    }
}
