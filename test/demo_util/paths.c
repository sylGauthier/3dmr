#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <game/asset_manager.h>

int register_asset_paths(const char* argv0) {
    char *ptr, *tmp;

    if ((ptr = strstr(argv0, "test/")) && !strchr(ptr + 5, '/')) {
        if (!(tmp = malloc((ptr - argv0) + 12))) {
            fprintf(stderr, "Error: memory allocation failed");
            return 0;
        }
        memcpy(tmp, argv0, (ptr - argv0) + 5);
        memcpy(tmp + (ptr - argv0) + 5, "assets", 7);
        asset_manager_add_path(tmp);
        if (ptr - argv0) {
            tmp[ptr - argv0] = 0;
        } else {
            strcpy(tmp, ".");
        }
        asset_manager_add_path(tmp);
        free(tmp);
    } else if (argv0[0] == '.' && argv0[1] == '/' && !strchr(argv0 + 2, '/')) {
        asset_manager_add_path("assets");
        asset_manager_add_path("..");
    } else {
        fprintf(stderr, "Error: unable to figure out asset paths\n");
        return 0;
    }

    return 1;
}

