#include <stdlib.h>
#include <string.h>

char* fullpath(const char* path, const char* file) {
    unsigned int len;
    char* res = NULL;

    if (path) {
        len = strlen(path) + strlen(file) + 2;
        if ((res = malloc(len))) {
            memcpy(res, path, strlen(path));
            res[strlen(path)] = '/';
            memcpy(res + strlen(path) + 1, file, strlen(file));
            res[len - 1] = '\0';
        }
    } else {
        if ((res = malloc(strlen(file) + 1))) {
            memcpy(res, file, strlen(file) + 1);
        }
    }
    return res;
}

