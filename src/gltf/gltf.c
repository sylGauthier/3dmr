#include "gltf.h"

int gltf_load(struct Node* root, FILE* gltfFile, const char* path, struct ImportSharedData* shared, struct ImportMetadata* metadata) {
    json_t *jroot;
    struct GltfContext context = {0};

    context.shared = shared;
    context.metadata = metadata;

    /* Save user structures to restore them as-is in case of failure */
    context.sharedSave = *shared;
    context.metadataSave = *metadata;

    context.path = path;
    context.root = root;
    if (!(jroot = json_loadf(gltfFile, 0, NULL))) {
        fprintf(stderr, "Error: gltf: could not load file\n");
        return 0;
    }

    if (       !gltf_parse_buffers(&context, jroot)
            || !gltf_parse_buffer_views(&context, jroot)
            || !gltf_parse_accessors(&context, jroot)
            || !gltf_parse_cameras(&context, jroot)
            || !gltf_parse_textures(&context, jroot)
            || !gltf_parse_materials(&context, jroot)
            || !gltf_parse_meshes(&context, jroot)
            || !gltf_parse_lights(&context, jroot)
            || !gltf_parse_nodes(&context, root, jroot)
            || !gltf_parse_skins(&context, jroot)
            || !gltf_parse_anims(&context, jroot)) {
        fprintf(stderr, "Error: gltf: failed\n");
        gltf_context_drop(&context);
        json_decref(jroot);
        return 0;
    }

    json_decref(jroot);
    gltf_context_free(&context);
    return 1;
}
