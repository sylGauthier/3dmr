#include <game/scene/import.h>
#include "axis.h"

#ifndef GAME_OPENGEX_CONTEXT_H
#define GAME_OPENGEX_CONTEXT_H

struct OgexContext {
    float scale, angle, time;
    enum OgexAxis up, forward;
    const char* path;

    struct ImportMetadata* metadata;

    struct OgexObjectList {
        void** objects;
        const struct ODDLStructure** structures;
        unsigned int num;
    } cameras, matParams, geometries, lights, boneNodes, skins;
    struct Clip** clips;
    unsigned int numClips;
};

void ogex_init_context(struct OgexContext* context);
void ogex_free_context(struct OgexContext* context);

void ogex_swap_yz_vec(Vec3 vec);
void ogex_swap_yz_mat(Mat4 mat);

int ogex_context_add_object(struct OgexObjectList* list, const struct ODDLStructure* ref, void* p);
void* ogex_context_find_object(const struct OgexObjectList* list, const struct ODDLStructure* ref);

#endif
