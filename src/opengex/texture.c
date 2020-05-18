#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <3dmr/render/texture.h>
#include "common.h"
#include "texture.h"

int ogex_parse_texture(const struct OgexContext* context, const struct ODDLStructure* cur, char** attrib, GLuint* tex) {
    struct ODDLStructure* sub;
    struct ODDLProperty* prop;
    unsigned int i, texSet = 0;

    if (!(prop = oddl_get_property(cur, "attrib"))) {
        fprintf(stderr, "Error: Texture: missing property \"attrib\"\n");
        return 0;
    }
    *attrib = prop->str;
    for (i = 0; i < cur->nbStructures; i++) {
        sub = cur->structures[i];
        if (sub->type == TYPE_STRING) {
            char* name;
            if (texSet) {
                fprintf(stderr, "Error: Texture: cannot have multiple textures\n");
                glDeleteTextures(1, tex);
                return 0;
            }
            if (!ogex_check_struct(sub, "Texture", TYPE_STRING, 1, 1)) return 0;
            name = ((char**)(sub->dataList))[0];
            if (name[0] == '/') {
                if (name[1] == '/') {
                    fprintf(stderr, "Error: Texture: //drive/path not supported\n");
                    return 0;
                }
                *tex = texture_load_from_png(name);
            } else if (!context->path) {
                *tex = texture_load_from_png(name);
            } else {
                char* path;
                size_t n = strlen(context->path), m = strlen(name), s;
                if (n > (((size_t)-1) - m) || (s = (n + m)) > ((size_t)-3) || !(path = malloc(s + 2))) {
                    fprintf(stderr, "Error: Texture: failed to allocated memory for texture path\n");
                    return 0;
                }
                memcpy(path, context->path, n);
                path[n++] = '/';
                memcpy(path + n, name, m);
                path[s + 1] = 0;
                *tex = texture_load_from_png(path);
                free(path);
            }
            if (!*tex) {
                fprintf(stderr, "Error: Texture: could not load texture file: %s\n", name);
                return 0;
            }
            texSet = 1;
        } else if (!sub->identifier) {
            continue;
        } else if (!strcmp(sub->identifier, "Transform")
                || !strcmp(sub->identifier, "Translation")
                || !strcmp(sub->identifier, "Rotation")
                || !strcmp(sub->identifier, "Scale")) {
            fprintf(stderr, "Warning: Texture: transformations are not supported yet\n");
        } else if (!strcmp(sub->identifier, "Animation")) {
            fprintf(stderr, "Warning: Texture: animations are not supported yet\n");
        }
    }
    if (!texSet) {
        fprintf(stderr, "Error: Texture: needs one filename\n");
        return 0;
    }
    return 1;
}
