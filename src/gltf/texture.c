#include <string.h>

#include <3dmr/render/texture.h>

#include "gltf.h"
#include "utils.h"

static int parse_img_file(struct GltfContext* context, GLuint* tex, json_t* juri) {
    char* pngPath;

    if (!(pngPath = fullpath(context->path, json_string_value(juri)))) {
        return 0;
    }
    if (!(*tex = texture_load_from_png(pngPath))) {
        fprintf(stderr, "Error: gltf: texture: could not load texture: %s\n", pngPath);
        free(pngPath);
        return 0;
    }
    free(pngPath);
    return 1;
}

static int parse_img_buffer(struct GltfContext* context, GLuint* tex, json_t* jbufview) {
    const char* data;
    unsigned int bufview;
    struct GltfBufferView* view;

    if (!json_is_integer(jbufview) || (bufview = json_integer_value(jbufview)) >= context->numBufferViews) {
        fprintf(stderr, "Error: gltf: texture: invalid bufferView idx\n");
        return 0;
    }
    view = &context->bufferViews[bufview];
    data = ((char*)context->buffers[view->buffer].data) + view->byteOffset;
    if (!(*tex = texture_load_from_png_buffer(data, view->byteLength))) {
        fprintf(stderr, "Error: gltf: texture: could not load texture from buffer\n");
        return 0;
    }
    return 1;
}

static int parse_img(struct GltfContext* context, GLuint* tex, json_t* jimg) {
    json_t* tmp;
    char pngMime = 0;

    if ((tmp = json_object_get(jimg, "mimeType"))) {
        if (!json_string_value(tmp) || strcmp(json_string_value(tmp), "image/png")) {
            fprintf(stderr, "Error: gltf: texture: invalid mimeType (only png supported)\n");
            return 0;
        }
        pngMime = 1;
    }
    if ((tmp = json_object_get(jimg, "uri"))) {
        switch (gltf_uri_type(tmp)) {
            case GLTF_URI_FILE:
                return parse_img_file(context, tex, tmp);
            case GLTF_URI_B64:
                fprintf(stderr, "Error: gltf: texture: base64 URIs for textures not implemented yet\n");
                return 0;
            default:
                fprintf(stderr, "Error: gltf: texture: invalid URI\n");
                return 0;
        }
    } else if ((tmp = json_object_get(jimg, "bufferView"))) {
        if (!pngMime) {
            fprintf(stderr, "Error: gltf: texture: missing mime type, required for buffer data\n");
            return 0;
        }
        return parse_img_buffer(context, tex, tmp);
    }
    return 1;
}

int gltf_parse_textures(struct GltfContext* context, json_t* jroot) {
    json_t *tex, *img, *curTex, *curImg;
    GLuint* texGLuint = NULL;
    unsigned int idx, numImages;

    if (!(tex = json_object_get(jroot, "textures"))
            || !json_array_size(tex)) {
        return 1;
    }
    if (!(img = json_object_get(jroot, "images"))
            || !json_array_size(img)) {
        fprintf(stderr, "Error: gltf: texture: no images\n");
        return 0;
    }
    numImages = json_array_size(img);
    context->numTextures = json_array_size(tex);
    if (!(texGLuint = malloc(numImages * sizeof(GLuint)))
            || !(context->textures = malloc(json_array_size(tex) * sizeof(GLuint)))) {
        fprintf(stderr, "Error: gltf: texture: could not allocate textures\n");
        free(texGLuint);
        return 0;
    }

    json_array_foreach(img, idx, curImg) {
        if (!parse_img(context, &texGLuint[idx], curImg)) {
            free(texGLuint);
            return 0;
        }
    }
    json_array_foreach(tex, idx, curTex) {
        json_t* tmp;
        if ((tmp = json_object_get(curTex, "sampler"))) {
            fprintf(stderr, "Warning: gltf: texture: sampler not supported yet\n");
        }
        if (!(tmp = json_object_get(curTex, "source"))) {
            fprintf(stderr, "Error: gltf: texture: missing source\n");
            free(texGLuint);
            return 0;
        }
        if (!json_is_integer(tmp) || json_integer_value(tmp) >= numImages) {
            fprintf(stderr, "Error: gltf: texture: invalid source index\n");
            free(texGLuint);
            return 0;
        }
        context->textures[idx] = texGLuint[json_integer_value(tmp)];
    }

    free(texGLuint);
    return 1;
}
