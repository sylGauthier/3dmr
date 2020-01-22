#include <stdlib.h>
#include <stdio.h>

#include <game/scene/import.h>

int import_add_shared_item(void* ptr, unsigned int* size, void* newItem) {
    void* tmp;
    void*** vec = ptr;

    if (!(tmp = realloc(*vec, (*size + 1) * sizeof(void*)))) {
        fprintf(stderr, "Error: import_add_shared_item: could not realloc mem for shared item\n");
        return 0;
    }
    *vec = tmp;
    (*vec)[*size] = newItem;
    (*size)++;
    return 1;
}

void import_free_shared_data(struct SharedData* shared) {
    unsigned int i;

    for (i = 0; i < shared->nbMat; i++) {
        free(shared->mats[i]);
    }
    for (i = 0; i < shared->nbMatParams; i++) {
        free(shared->matParams[i]);
    }
    for (i = 0; i < shared->nbVA; i++) {
        vertex_array_free(shared->va[i]);
    }
    for (i = 0; i < shared->nbCams; i++) {
        free(shared->cams[i]);
    }
    for (i = 0; i < shared->nbPLights; i++) {
        free(shared->plights[i]);
    }
    for (i = 0; i < shared->nbDLights; i++) {
        free(shared->dlights[i]);
    }
    for (i = 0; i < shared->nbSkins; i++) {
        skin_free(shared->skins[i]);
    }
    free(shared->mats);
    free(shared->matParams);
    free(shared->va);
    free(shared->cams);
    free(shared->plights);
    free(shared->dlights);
    free(shared->skins);
}

void import_free_metadata(struct ImportMetadata* metadata) {
    unsigned int i;

    free(metadata->cameraNodes);
    free(metadata->lightNodes);
    for (i = 0; i < metadata->nbClips; i++) {
        anim_free_clip(metadata->clips + i);
    }
    free(metadata->clips);
}
