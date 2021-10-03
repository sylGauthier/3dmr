#include "gltf.h"

int gltf_load(struct Node* root, FILE* gltfFile, const char* path, struct ImportSharedData* shared, struct ImportMetadata* metadata, struct ImportOptions* opts) {
    json_t *jroot;
    json_error_t error;
    struct GltfContext context = {0};

    context.shared = shared;
    context.metadata = metadata;

    /* Save user structures to restore them as-is in case of failure */
    context.sharedSave = *shared;
    context.metadataSave = *metadata;
    context.nbChildrenSave = root->nbChildren;

    context.path = path;
    context.file = gltfFile;
    context.opts = opts;
    context.binary = opts && (opts->flags & TDMR_IMPORT_BINARY);
    context.root = root;
    if (context.binary) {
        uint32_t header[5];
        if (fread(header, 4, 5, gltfFile) != 5) {
            fprintf(stderr, "Error: gltf: binary file: couldn't read header\n");
            return 0;
        }
        if (header[0] != 0x46546C67) {
            fprintf(stderr, "Error: gltf: binary file: invalid magic\n");
            return 0;
        } else if (header[1] != 2) {
            fprintf(stderr, "Error: gltf: binary file: invalid glTF version (expected 2)\n");
            return 0;
        } else if (header[4] != 0x4E4F534A) {
            fprintf(stderr, "Error: gltf: binary file: invalid chunk type (expected JSON)\n");
            return 0;
        }
    }
    if (!(jroot = json_loadf(gltfFile, JSON_DISABLE_EOF_CHECK, &error))) {
        fprintf(stderr, "Error: gltf: could not load file, JSON error:\n");
        fprintf(stderr, "%s\n", error.text);
        return 0;
    }
    if (context.binary) {
        long pos = ftell(gltfFile);
        uint32_t header[2];
        /* Align correctly for the rest of the data */
        if (pos % 4) {
            if (fseek(gltfFile, 4 - pos % 4, SEEK_CUR)) {
                fprintf(stderr, "Error: gltf: binary file: missing padding after JSON data\n");
                return 0;
            }
        }
        if (fread(header, 4, 2, gltfFile) == 2) {
            if (header[1] != 0x004E4942) {
                fprintf(stderr, "Error: gltf: binary file: invalid chunk type (expected BIN)\n");
                return 0;
            }
        }
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
