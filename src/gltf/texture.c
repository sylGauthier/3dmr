#include <string.h>

#include <3dmr/render/texture.h>

#include "gltf.h"
#include "utils.h"

enum ImgMime {
    IMG_MIME_PNG,
    IMG_MIME_JPEG,
    IMG_MIME_UNDEF
};

static enum ImgMime infer_mime(const char* filename) {
    const char* ptr;
    if ((ptr = strrchr(filename, '.'))) {
        if (!strcmp(ptr, ".png")) return IMG_MIME_PNG;
        if (!strcmp(ptr, ".jpeg")) return IMG_MIME_JPEG;
        if (!strcmp(ptr, ".JPEG")) return IMG_MIME_JPEG;
        if (!strcmp(ptr, ".jpg")) return IMG_MIME_JPEG;
        if (!strcmp(ptr, ".JPG")) return IMG_MIME_JPEG;
    }
    return IMG_MIME_UNDEF;
}

static int parse_img_file(struct GltfContext* context, GLuint* tex, json_t* juri, enum ImgMime mime) {
    char* path;
    GLuint (*texture_load)(const char*);

    if (!(path = fullpath(context->path, json_string_value(juri)))) {
        return 0;
    }
    if (mime == IMG_MIME_UNDEF) mime = infer_mime(path);
    switch (mime) {
        case IMG_MIME_PNG:
            texture_load = texture_load_from_png;
            break;
        case IMG_MIME_JPEG:
            texture_load = texture_load_from_jpeg;
            break;
        default:
            fprintf(stderr, "Error: gltf: texture: can't infer mime type for %s, unkown extension\n", path);
            return 0;
    }
    if (!(*tex = texture_load(path))) {
        fprintf(stderr, "Error: gltf: texture: could not load texture: %s\n", path);
        free(path);
        return 0;
    }
    free(path);
    return 1;
}

static int parse_img_buffer(struct GltfContext* context, GLuint* tex, json_t* jbufview, enum ImgMime mime) {
    const char* data;
    unsigned int bufview;
    struct GltfBufferView* view;

    if (!json_is_integer(jbufview) || (bufview = json_integer_value(jbufview)) >= context->numBufferViews) {
        fprintf(stderr, "Error: gltf: texture: invalid bufferView idx\n");
        return 0;
    }
    view = &context->bufferViews[bufview];
    data = ((char*)context->buffers[view->buffer].data) + view->byteOffset;
    switch (mime) {
        case IMG_MIME_PNG:
            if (!(*tex = texture_load_from_png_buffer(data, view->byteLength))) {
                fprintf(stderr, "Error: gltf: texture: could not load texture from buffer\n");
                return 0;
            }
            break;
        case IMG_MIME_JPEG:
            if (!(*tex = texture_load_from_jpeg_buffer(data, view->byteLength))) {
                fprintf(stderr, "Error: gltf: texture: could not load texture from buffer\n");
                return 0;
            }
            break;
        default:
            fprintf(stderr, "Error: gltf: texture: missing mime type, required for buffer data\n");
            return 0;
    }
    return 1;
}

static int parse_img(struct GltfContext* context, GLuint* tex, json_t* jimg) {
    json_t* tmp;
    enum ImgMime mime = IMG_MIME_UNDEF;

    if ((tmp = json_object_get(jimg, "mimeType"))) {
        if (!json_string_value(tmp)) {
            fprintf(stderr, "Error: gltf: texture: invalid mimeType.\n");
            return 0;
        }
        if (!strcmp(json_string_value(tmp), "image/png")) {
            mime = IMG_MIME_PNG;
        } else if (!strcmp(json_string_value(tmp), "image/jpeg")) {
            mime = IMG_MIME_JPEG;
        } else {
            fprintf(stderr, "Error: gltf: texture: unsupported mimeType: %s\n", json_string_value(tmp));
            return 0;
        }
    }
    if ((tmp = json_object_get(jimg, "uri"))) {
        switch (gltf_uri_type(tmp)) {
            case GLTF_URI_FILE:
                return parse_img_file(context, tex, tmp, mime);
            case GLTF_URI_B64:
                fprintf(stderr, "Error: gltf: texture: base64 URIs for textures not implemented yet\n");
                return 0;
            default:
                fprintf(stderr, "Error: gltf: texture: invalid URI\n");
                return 0;
        }
    } else if ((tmp = json_object_get(jimg, "bufferView"))) {
        return parse_img_buffer(context, tex, tmp, mime);
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
