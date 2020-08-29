#include <string.h>

#include "gltf.h"
#include "utils.h"

static const char* b64hint = "data:application/octet-stream;base64,";

/* TODO: be compliant with RFC: https://tools.ietf.org/html/rfc2397 */

enum GltfURIType gltf_uri_type(json_t* juri) {
    const char* uritxt;

    if (!(uritxt = json_string_value(juri))) {
        fprintf(stderr, "Error: gltf: invalid uri type (should be string)\n");
        return GLTF_URI_NONE;
    }
    if (strlen(uritxt) >= strlen(b64hint) && !strncmp(uritxt, b64hint, strlen(b64hint)))
        return GLTF_URI_B64;
    return GLTF_URI_FILE;
}

static int load_file_uri(struct GltfContext* context, const char* filename, struct GltfBuffer* dest) {
    char* path = NULL;
    FILE* f = NULL;
    char ok = 1;

    if (!(path = fullpath(context->path, filename))) {
        fprintf(stderr, "Error: gltf: uri: filename error\n");
        ok = 0;
    } else if (!(f = fopen(path, "r"))) {
        fprintf(stderr, "Error: gltf: could not open buffer file: %s\n", path);
        ok = 0;
    } else if (fread(dest->data, 1, dest->size, f) != dest->size) {
        fprintf(stderr, "Error: gltf: size of file: %s doesn't match with buffer size (expected %d)\n", path, dest->size);
        ok = 0;
    }
    free(path);
    if (f) fclose(f);
    return ok;
}

static int load_b64_chunk(char* dest, const char* src, unsigned int size) {
    uint8_t s[4];
    unsigned int i;

    for (i = 0; i < 4; i++) {
        if (src[i] >= 'A' && src[i] <= 'Z') {
            s[i] = src[i] - 'A';
        } else if (src[i] >= 'a' && src[i] <= 'z') {
            s[i] = src[i] - 'a' + 26;
        } else if (src[i] >= '0' && src[i] <= '9') {
            s[i] = src[i] - '0' + 52;
        } else if (src[i] == '+') {
            s[i] = 62;
        } else if (src[i] == '/') {
            s[i] = 63;
        } else if (src[i] == '=') {
            s[i] = 0;
        } else {
            printf("Error: gltf: uri: base64: invalid character: %c\n", src[i]);
            return 0;
        }
    }
    switch (size) {
        case 3: dest[2] = (s[2] << 6) + (s[3] >> 0);
        case 2: dest[1] = (s[1] << 4) + (s[2] >> 2);
        case 1: dest[0] = (s[0] << 2) + (s[1] >> 4);
        default: break;
    }
    return 1;
}

/* This function assumes that the text buffer starts with the b64hint string, otherwise it will crash */
static int load_b64_uri(struct GltfContext* context, const char* txt, struct GltfBuffer* dest) {
    const char* buf = txt + strlen(b64hint);
    char* data = dest->data;
    unsigned int txtLen = strlen(buf), i;

    if (txtLen && txtLen / 4 - dest->size / 3 > 1) {
        fprintf(stderr, "Error: gltf: uri: base64 data length doesn't match buffer size\n");
        return 0;
    }
    if (txtLen % 4) {
        fprintf(stderr, "Error: gltf: uri: base64 buffer should have a length multiple of 4\n");
        return 0;
    }
    for (i = 0; i < txtLen / 4; i++) {
        if (!load_b64_chunk(data + 3 * i, buf + 4 * i, dest->size - 3 * i > 3 ? 3 : dest->size - 3 * i))
            return 0;
    }
    return 1;
}

int gltf_load_uri(struct GltfContext* context, json_t* juri, struct GltfBuffer* dest) {
    switch (gltf_uri_type(juri)) {
        case GLTF_URI_FILE:
            return load_file_uri(context, json_string_value(juri), dest);
        case GLTF_URI_B64:
            return load_b64_uri(context, json_string_value(juri), dest);
        default:
            break;
    }
    return 0;
}
