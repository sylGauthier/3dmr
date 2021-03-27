#include <string.h>

#include "gltf.h"

static unsigned int* acc_get_uint(struct GltfContext* context, struct GltfAccessor* acc) {
    struct GltfBufferView* view;
    struct GltfBuffer* buffer;
    void* data;
    uint16_t* data8;
    uint16_t* data16;
    uint32_t* data32;
    unsigned int *res, compSize;

    if (acc->type == GLTF_SCALAR) {
        compSize = 1;
    } else if (acc->type == GLTF_VEC2) {
        compSize = 2;
    } else if (acc->type == GLTF_VEC3) {
        compSize = 3;
    } else if (acc->type == GLTF_VEC4) {
        compSize = 4;
    } else {
        return NULL;
    }
    if (!(acc->componentType == GLTF_UNSIGNED_SHORT
                || acc->componentType == GLTF_UNSIGNED_BYTE
                || acc->componentType == GLTF_UNSIGNED_INT)) {
        fprintf(stderr, "Error: gltf: mesh: unsupported indices componentType (should be UNSIGNED_SHORT or UNSIGNED_INT)\n");
        return NULL;
    }
    if (!(res = malloc(compSize * acc->count * sizeof(*res)))) {
        fprintf(stderr, "Error: gltf: can't allocate memory\n");
        return NULL;
    }
    view = &context->bufferViews[acc->bufferView];
    buffer = &context->buffers[view->buffer];
    data = ((char*) buffer->data) + view->byteOffset;
    if (acc->componentType == GLTF_UNSIGNED_SHORT) {
        unsigned int i;
        data16 = data;
        for (i = 0; i < compSize * acc->count; i++) {
            res[i] = data16[i];
        }
    } else if (acc->componentType == GLTF_UNSIGNED_INT) {
        unsigned int i;
        data32 = data;
        for (i = 0; i < compSize * acc->count; i++) {
            res[i] = data32[i];
        }
    } else if (acc->componentType == GLTF_UNSIGNED_BYTE) {
        unsigned int i;
        data8 = data;
        for (i = 0; i < compSize * acc->count; i++) {
            res[i] = data8[i];
        }
    }
    return res;
}

static int build_mesh(struct GltfContext* context, struct Mesh* mesh,
                      unsigned int* indices, unsigned int numIndices,
                      float* positions,
                      float* normals,
                      float* texCoords,
                      unsigned int* joints,
                      float* weights,        unsigned int numVertices) {
    unsigned int i, offset = 0;

    if (indices) {
        mesh->indices = indices;
        mesh->numIndices = numIndices;
    }
    if (!(mesh->vertices = malloc(numVertices * MESH_FLOATS_PER_VERTEX(mesh) * sizeof(float)))) {
        fprintf(stderr, "Error: gltf: mesh: can't allocate memory for vertices\n");
        return 0;
    }
    mesh->numVertices = numVertices;
    for (i = 0; i < numVertices; i++) {
        memcpy(mesh->vertices + i * MESH_FLOATS_PER_VERTEX(mesh), positions + 3 * i, sizeof(Vec3));
    }
    offset += 3;
    if (normals) {
        for (i = 0; i < numVertices; i++) {
            memcpy(mesh->vertices + i * MESH_FLOATS_PER_VERTEX(mesh) + offset, normals + 3 * i, sizeof(Vec3));
        }
        offset += 3;
    }
    if (texCoords) {
        for (i = 0; i < numVertices; i++) {
            mesh->vertices[i * MESH_FLOATS_PER_VERTEX(mesh) + offset] = texCoords[2 * i];
            mesh->vertices[i * MESH_FLOATS_PER_VERTEX(mesh) + offset + 1] = 1 - texCoords[2 * i + 1];
        }
        offset += 2;
    }
    if (joints && weights) {
        for (i = 0; i < numVertices; i++) {
            float maxIdx1 = 0, maxIdx2 = 0;
            float maxWgt1 = 0, maxWgt2 = 0;
            unsigned int j;

            for (j = 0; j < 4; j++) {
                if (weights[4 * i + j] >= maxWgt1) {
                    maxIdx1 = joints[4 * i + j];
                    maxWgt1 = weights[4 * i + j];
                } else if (weights[4 * i + j] >= maxWgt2) {
                    maxIdx2 = joints[4 * i + j];
                    maxWgt2 = weights[4 * i + j];
                }
            }
            mesh->vertices[i * MESH_FLOATS_PER_VERTEX(mesh) + offset] = maxIdx1;
            mesh->vertices[i * MESH_FLOATS_PER_VERTEX(mesh) + offset + 1] = maxIdx2;
            mesh->vertices[i * MESH_FLOATS_PER_VERTEX(mesh) + offset + 2] = maxWgt1;
            mesh->vertices[i * MESH_FLOATS_PER_VERTEX(mesh) + offset + 3] = maxWgt2;
        }
        offset += 4;
    }
    return 1;
}

int gltf_parse_meshes(struct GltfContext* context, json_t* jroot) {
    json_t *meshes, *curMesh;
    unsigned int idx;

    if (!(meshes = json_object_get(jroot, "meshes"))) {
        return 1;
    }
    if (!(context->numMeshes = json_array_size(meshes))) {
        return 1;
    }
    if (!(context->meshes = calloc(context->numMeshes, sizeof(void*)))) {
        fprintf(stderr, "Error: gltf: meshes: can't allocate memory for meshes\n");
        return 0;
    }
    json_array_foreach(meshes, idx, curMesh) {
        json_t *primitives, *attributes, *tmp;
        struct GltfAccessor* acc;

        struct PBRMaterialParams* pbr = NULL;
        struct Mesh mesh = {0};
        struct Geometry* geom = NULL;

        unsigned int *indices = NULL, *joints = NULL, numIndices = 0, numVertices = 0;
        float *pos = NULL, *normals = NULL, *texCoords = NULL, *weights = NULL;
        char ok = 0;

        if (!(context->meshes[idx] = calloc(1, sizeof(**context->meshes)))) {
            fprintf(stderr, "Error: gltf: mesh: can't allocate geometry\n");
            return 0;
        }

        /* get primitives */
        if (!(primitives = json_object_get(curMesh, "primitives")) || !json_array_size(primitives)) {
            fprintf(stderr, "Error: gltf: mesh: missing primitives\n");
            return 0;
        }
        if (json_array_size(primitives) != 1) {
            fprintf(stderr, "Warning: gltf: mesh: multiple primitives not supported\n");
        }
        primitives = json_array_get(primitives, 0);

        /* get indice array */
        if ((tmp = json_object_get(primitives, "indices"))) {
            if (!(acc = gltf_get_acc(context, json_integer_value(tmp)))) return 0;
            if (!(indices = acc_get_uint(context, acc))) return 0;
            numIndices = acc->count;
        }

        /* get material */
        if ((tmp = json_object_get(primitives, "material"))) {
            if (json_integer_value(tmp) >= context->numMaterials) {
                fprintf(stderr, "Error: gltf: mesh: invalid material index\n");
                return 0;
            }
            pbr = context->materials[json_integer_value(tmp)];
        } else if (!(pbr = pbr_material_params_new())) {
            fprintf(stderr, "Error: gltf: mesh: can't create default material\n");
            return 0;
        }

        /* get position, normal, texCoord arrays */
        if (!(attributes = json_object_get(primitives, "attributes"))) {
            fprintf(stderr, "Error: gltf: mesh: missing attributes\n");
            return 0;
        }

        if (!(tmp = json_object_get(attributes, "POSITION"))
                || !(acc = gltf_get_acc(context, json_integer_value(tmp)))
                || acc->type != GLTF_VEC3
                || acc->componentType != GLTF_FLOAT
                || !(pos = gltf_acc_get_buf(context, acc))) {
            fprintf(stderr, "Error: gltf: mesh: needs at least POSITION attribute\n");
            return 0;
        }
        numVertices = acc->count;

        if ((tmp = json_object_get(attributes, "NORMAL"))) {
            if ((acc = gltf_get_acc(context, json_integer_value(tmp)))
                    && acc->type == GLTF_VEC3
                    && acc->componentType == GLTF_FLOAT
                    && acc->count == numVertices
                    && (normals = gltf_acc_get_buf(context, acc))) {
                mesh.flags |= MESH_NORMALS;
            } else {
                fprintf(stderr, "Warning: gltf: mesh: discarding invalid normals\n");
            }
        }
        if ((tmp = json_object_get(attributes, "TEXCOORD_0"))) {
            if ((acc = gltf_get_acc(context, json_integer_value(tmp)))
                    && acc->type == GLTF_VEC2
                    && acc->componentType == GLTF_FLOAT
                    && acc->count == numVertices
                    && (texCoords = gltf_acc_get_buf(context, acc))) {
                mesh.flags |= MESH_TEXCOORDS;
            } else {
                fprintf(stderr, "Warning: gltf: mesh: discarding invalid texCoords\n");
            }
        }
        if ((tmp = json_object_get(attributes, "JOINTS_0"))) {
            if ((acc = gltf_get_acc(context, json_integer_value(tmp)))
                    && acc->count == numVertices
                    && (joints = acc_get_uint(context, acc))) {
                if (!(tmp = json_object_get(attributes, "WEIGHTS_0"))) {
                    fprintf(stderr, "Warning: gltf: JOINTS_0 defined but no WEIGHTS_0, discarding skin\n");
                } else {
                    if ((acc = gltf_get_acc(context, json_integer_value(tmp)))
                            && acc->type == GLTF_VEC4
                            && acc->componentType == GLTF_FLOAT
                            && acc->count == numVertices
                            && (weights = gltf_acc_get_buf(context, acc))) {
                        mesh.flags |= MESH_SKIN;
                    } else {
                        fprintf(stderr, "Warning: gltf: mesh: discarding invalid weights\n");
                    }
                }
            } else {
                fprintf(stderr, "Warning: gltf: mesh: discarding invalid joints\n");
            }
        }

        /* concatenate arrays and build up a Mesh structure */
        if (!build_mesh(context, &mesh, indices, numIndices, pos, normals, texCoords, joints, weights, numVertices)) {
            fprintf(stderr, "Error: gltf: mesh: could not build mesh\n");
        } else if (geom = context->meshes[idx],
                   !(geom->vertexArray = vertex_array_new(&mesh))
                || !import_add_shared_va(context->shared, geom->vertexArray)) {
            fprintf(stderr, "Error: gltf: mesh: could not create VertexArray\n");
        } else if (!(geom->material = pbr_material_new(mesh.flags, pbr))) {
            fprintf(stderr, "Error: gltf: mesh: could not create material\n");
        } else if (!import_add_shared_geometry(context->shared, geom)) {
            fprintf(stderr, "Error: gltf: mesh: could not add shared geometry\n");
        } else {
            ok = 1;
        }

        if (geom) mesh_free(&mesh);
        free(joints);
        if (!ok) return 0;
    }
    return 1;
}
