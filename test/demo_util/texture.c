#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <game/asset_manager.h>
#include <game/render/texture.h>
#include "args.h"
#include "checkerboard.h"

char* test_texture_path(const char* name) {
    char *path, *realpath;

    if (!(path = malloc(14 + strlen(name)))) {
        fprintf(stderr, "Error: failed to allocate memory for texture path '%s'\n", name);
        return NULL;
    }
    sprintf(path, "textures/%s.png", name);
    realpath = asset_manager_find_file(path);
    free(path);
    if (!realpath) {
        fprintf(stderr, "Error: failed to find texture '%s'\n", name);
    }
    return realpath;
}

GLuint test_texture(const char* name) {
    char* path;
    GLuint res;

    if (!strncmp(name, "color:", 6)) {
        Vec3 color;
        unsigned char buffer[4];
        if (parse_vec3(name + 6, NULL, color)) {
            buffer[0] = color[0] * 255;
            buffer[1] = color[1] * 255;
            buffer[2] = color[2] * 255;
            buffer[3] = 1;
            return texture_load_from_buffer(buffer, 1, 1, 1);
        }
        fprintf(stderr, "Error: failed to parse texture color\n");
        return 0;
    }
    
    if (!strncmp(name, "checkerboard", 12) && (!name[12] || name[12] == ':')) {
        if (name[12]) {
            Vec4 color1, color2;
            if (parse_vec3(name + 13, (char**)&name, color1)
             && *name++ == ','
             && parse_vec3(name, (char**)&name, color2)
             && !*name) {
                color1[3] = 1;
                color2[3] = 1;
                return texture_checkerboard(color1, color2);
            }
            fprintf(stderr, "Warning: invalid checkerboard color specification, falling back to default settings\n");
        }
        return default_checkerboard();
    }

    if (!(path = test_texture_path(name))) {
        return 0;
    }
    res = texture_load_from_file(path);
    free(path);
    return res;
}
