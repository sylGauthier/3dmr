#include <string.h>

#include <3dmr/render/texture.h>

#include "gltf.h"
#include "utils.h"

int parse_img(GLuint* tex, json_t* jimg, const char* path) {
    json_t* tmp;
    char* pngPath;

    if ((tmp = json_object_get(jimg, "mimeType"))) {
        if (!json_string_value(tmp) || strcmp(json_string_value(tmp), "image/png")) {
            fprintf(stderr, "Error: gltf: texture: invalid mimeType (only png supported)\n");
            return 0;
        }
    }
    if (!(tmp = json_object_get(jimg, "uri"))
            || !json_string_value(tmp)) {
        fprintf(stderr, "Error: gltf: texture: missing uri\n");
        return 0;
    }
    if (!(pngPath = fullpath(path, json_string_value(tmp)))) {
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
        if (!parse_img(&texGLuint[idx], curImg, context->path)) {
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
