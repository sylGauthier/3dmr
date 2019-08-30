#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <game/animation/animation.h>

void clip_init(struct Clip* clip) {
    clip->duration = 0;
    clip->mode = CLIP_FORWARD;
    clip->loop = 0;

    clip->nbAnimations = 0;
    clip->animations = 0;

    clip->curPos = 0;
    clip->rev = 0;
}

static void init_animation(struct Animation* anim) {
    unsigned int i;

    anim->targetNode = NULL;
    for (i = 0; i < TRACK_NB_TYPES; i++) {
        anim->tracks[i].times.values = NULL;
        anim->tracks[i].values.values = NULL;
        anim->tracks[i].nbKeys = 0;
        anim->tracks[i].lastIdx = 0;
    }
}

int clip_new_anim(struct Clip* clip, struct Node* targetNode) {
    void* tmp;

    if (!(tmp = realloc(clip->animations, (clip->nbAnimations + 1) * sizeof(struct Animation)))) {
        fprintf(stderr, "Error: clip_new_anim: could not reallocate memory for new animation\n");
        return -1;
    }
    clip->animations = tmp;
    init_animation(clip->animations + clip->nbAnimations);
    return clip->nbAnimations++;
}

int anim_new_slot(struct AnimationEngine* engine) {
    void* tmp;

    if (!(tmp = realloc(engine->animQueue, (engine->nbAnimSlots + 1) * sizeof(struct AnimStack)))) {
        fprintf(stderr, "Error: anim_new_slot: could not reallocate memory for new anim slot\n");
        return -1;
    }
    engine->animQueue = tmp;
    engine->animQueue[engine->nbAnimSlots] = NULL;
    return engine->nbAnimSlots++;
}

static void init_anim_stack(struct AnimStack* stack) {
    stack->clip = NULL;
    stack->delay = 0;
    stack->nextInStack = NULL;
}

int anim_append_clip(struct AnimationEngine* engine, struct Clip* clip, unsigned int slot, unsigned int delay) {
    struct AnimStack* newElem;
    struct AnimStack* cur;

    if (!(newElem = malloc(sizeof(*newElem)))) {
        fprintf(stderr, "Error: anim_append_clip: could not allocate memory for new animStack elem\n");
        return 0;
    }

    init_anim_stack(newElem);
    newElem->clip = clip;
    newElem->delay = delay;

    if (!(cur = engine->animQueue[slot])) {
        engine->animQueue[slot] = newElem;
        return 1;
    }
    while (cur->nextInStack) cur = cur->nextInStack;
    cur->nextInStack = newElem;
    return 1;
}

int anim_push_clip(struct AnimationEngine* engine, struct Clip* clip, unsigned int slot, unsigned int delay) {
    struct AnimStack* newElem;

    if (!(newElem = malloc(sizeof(*newElem)))) {
        fprintf(stderr, "Error: anim_append_clip: could not allocate memory for new animStack elem\n");
        return 0;
    }

    init_anim_stack(newElem);
    newElem->clip = clip;
    newElem->delay = delay;
    newElem->nextInStack = engine->animQueue[slot];
    engine->animQueue[slot] = newElem;
    return 1;
}

