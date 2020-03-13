#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "context.h"
#include "clip.h"
#include "name.h"

struct Clip* ogex_create_new_clip(struct OgexContext* context, unsigned int index) {
    struct Clip* clip;

    if (index >= ((unsigned int)-1)) {
        fprintf(stderr, "Error: Clip: invalid clip idx: %u\n", index);
        return NULL;
    }
    if (context->numClips <= index) {
        struct Clip** tmp;
        unsigned int i;
        if (!(tmp = realloc(context->clips, (index + 1) * sizeof(*context->clips)))) {
            fprintf(stderr, "Error: Clip: failed to allocate clip\n");
            return NULL;
        }
        for (i = context->numClips; i <= index; i++) {
            tmp[i] = NULL;
        }
        context->clips = tmp;
        context->numClips = index + 1;
    }
    if (context->clips[index]) {
        fprintf(stderr, "Error: Clip: clip #%u already exists\n", index);
        return NULL;
    }
    if (!(clip = context->clips[index] = malloc(sizeof(*clip)))) {
        fprintf(stderr, "Error: Clip: failed to allocate clip\n");
        return NULL;
    }
    anim_clip_init(clip);
    return clip;
}

struct Clip* ogex_parse_clip(struct OgexContext* context, const struct ODDLStructure* cur) {
    struct Clip* clip;
    struct ODDLProperty* prop;
    unsigned int clipIdx = 0, i;
    char* name = NULL;

    if ((prop = oddl_get_property(cur, "index"))) {
        clipIdx = prop->llint;
    }

    for (i = 0; i < cur->nbStructures; i++) {
        if (!cur->structures[i]->identifier) {
            continue;
        } else if (!strcmp(cur->structures[i]->identifier, "Param")) {
            fprintf(stderr, "Warning: Clip: Params are not supported\n");
        } else if (!strcmp(cur->structures[i]->identifier, "Name")) {
            free(name);
            if (!(name = ogex_parse_name(cur->structures[i]))) {
                fprintf(stderr, "Warning: Clip: could not parse name\n");
            }
        }
    }
    if (!(clip = ogex_create_new_clip(context, clipIdx))) return NULL;
    clip->name = name;
    return clip;
}
