#ifndef TDMR_GLTF_H
#define TDMR_GLTF_H

#include <stdlib.h>
#include <jansson.h>

#include <3dmr/scene/gltf.h>
#include <3dmr/material/pbr.h>

struct GltfLight {
    enum GltfLightType {
        GLTF_POINT,         /* "point" */
        GLTF_DIRECTIONAL,   /* "directional" */
        GLTF_SPOT           /* "spot" */
    } type;
    union Light_ {
        struct DirectionalLight dir;
        struct PointLight point;
        struct SpotLight spot;
    } light;
};

struct GltfBuffer {
    void* data;
    unsigned int size;
};

struct GltfBufferView {
    unsigned int buffer;
    unsigned int byteLength;
    unsigned int byteOffset;
};

enum GltfComponentType {
    GLTF_BYTE           = 5120,
    GLTF_UNSIGNED_BYTE  = 5121,
    GLTF_SHORT          = 5122,
    GLTF_UNSIGNED_SHORT = 5123,
    GLTF_UNSIGNED_INT   = 5125,
    GLTF_FLOAT          = 5126
};

enum GltfType {
    GLTF_SCALAR,
    GLTF_VEC2,
    GLTF_VEC3,
    GLTF_VEC4,
    GLTF_MAT2,
    GLTF_MAT3,
    GLTF_MAT4
};

struct GltfAccessor {
    unsigned int bufferView;
    unsigned int byteOffset;
    unsigned int count;
    enum GltfComponentType componentType;
    enum GltfType type;
};

struct GltfContext {
    struct Node** nodes;
    unsigned int numNodes;

    struct Geometry** meshes;
    unsigned int numMeshes;

    struct PBRMaterialParams** materials;
    unsigned int numMaterials;

    GLuint* textures;
    unsigned int numTextures;

    struct Camera** cameras;
    unsigned int numCameras;

    struct Skin** skins;
    unsigned int numSkins;

    struct Clip** clips;
    unsigned int numClips;


    struct GltfLight* lights;
    unsigned int numLights;

    struct GltfBuffer* buffers;
    unsigned int numBuffers;

    struct GltfBufferView* bufferViews;
    unsigned int numBufferViews;

    struct GltfAccessor* accessors;
    unsigned int numAccessors;

    struct ImportSharedData *shared, sharedSave;
    struct ImportMetadata *metadata, metadataSave;
    struct Node* root;
    const char* path;
};

int gltf_parse_buffers(struct GltfContext* context, json_t* jroot);
int gltf_parse_buffer_views(struct GltfContext* context, json_t* jroot);
int gltf_parse_accessors(struct GltfContext* context, json_t* jroot);

int gltf_parse_cameras(struct GltfContext* context, json_t* jroot);
int gltf_parse_textures(struct GltfContext* context, json_t* jroot);
int gltf_parse_materials(struct GltfContext* context, json_t* jroot);
int gltf_parse_meshes(struct GltfContext* context, json_t* jroot);
int gltf_parse_lights(struct GltfContext* context, json_t* jroot);
int gltf_parse_skins(struct GltfContext* context, json_t* jroot);
int gltf_parse_nodes(struct GltfContext* context, struct Node* root, json_t* jroot);
int gltf_parse_anims(struct GltfContext* context, json_t* jroot);

struct GltfAccessor* gltf_get_acc(struct GltfContext* context, unsigned int idx);
void* gltf_acc_get_buf(struct GltfContext* context, struct GltfAccessor* acc);
int gltf_get_root_nodes(json_t* nodes, unsigned int** roots, unsigned int* numRoots);

void gltf_context_free(struct GltfContext* context);
void gltf_context_drop(struct GltfContext* context);

#endif
