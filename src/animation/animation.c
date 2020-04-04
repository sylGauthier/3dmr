#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <game/animation/animation.h>

int anim_curve_init(struct AnimCurve* curve, enum TrackCurve type, unsigned int n) {
    curve->sharedValues = 0;
    switch (curve->curveType = type) {
        case TRACK_CURVE_NONE: return 1;
        case TRACK_LINEAR: return n < ((unsigned int)-1) / sizeof(float) && (curve->values.linear = malloc(n * sizeof(float)));
        case TRACK_BEZIER: return n < ((unsigned int)-1) / sizeof(Vec3) && (curve->values.bezier = malloc(n * sizeof(Vec3)));
    }
    return 0;
}

void anim_curve_free(struct AnimCurve* curve) {
    if (curve->sharedValues) return;
    switch (curve->curveType) {
        case TRACK_LINEAR: free(curve->values.linear); break;
        case TRACK_BEZIER: free(curve->values.bezier); break;
        default:;
    }
}

void anim_curve_copy(struct AnimCurve* dest, const struct AnimCurve* src) {
    dest->sharedValues = 1;
    dest->curveType = src->curveType;
    dest->values = src->values;
}

int anim_track_init(struct Track* track, enum TrackCurve timeCurve, enum TrackCurve valCurve, unsigned int numKeys) {
    if (!anim_curve_init(&track->times, timeCurve, numKeys)) return 0;
    if (!anim_curve_init(&track->values, valCurve, numKeys)) {
        anim_curve_free(&track->times);
        return 0;
    }
    track->numKeys = numKeys;
    track->lastIdx = 0;
    return 1;
}

void anim_track_free(struct Track* track) {
    anim_curve_free(&track->times);
    anim_curve_free(&track->values);
}

struct Track* anim_new_track_set(void) {
    struct Track* t;
    unsigned int i;

    if (!(t = malloc(TRACK_NB_TYPES * sizeof(*t)))) return 0;
    for (i = 0; i < TRACK_NB_TYPES; i++) {
        anim_track_init(t + i, TRACK_CURVE_NONE, TRACK_CURVE_NONE, 0); /* cannot fail with TRACK_CURVE_NONE */
    }
    return t;
}

int anim_animation_init(struct Animation* anim, struct Node* targetNode) {
    anim->targetNode = targetNode;
    if (!(anim->tracks = anim_new_track_set())) return 0;
    anim->flags = 0;
    return 1;
}

void anim_animation_free(struct Animation* anim) {
    if (anim->tracks) {
        unsigned int i;
        for (i = 0; i < TRACK_NB_TYPES; i++) {
            anim_track_free(&anim->tracks[i]);
        }
        free(anim->tracks);
    }
}

void anim_clip_init(struct Clip* clip) {
    clip->duration = 0;
    clip->mode = CLIP_FORWARD;
    clip->loop = 0;

    clip->numAnimations = 0;
    clip->animations = NULL;

    clip->curPos = 0;
    clip->rev = 0;
    clip->oneShot = 0;
    clip->name = NULL;
}

void anim_clip_free(struct Clip* clip) {
    unsigned int i;
    for (i = 0; i < clip->numAnimations; i++) {
        anim_animation_free(clip->animations + i);
    }
    free(clip->animations);
    free(clip->name);
}

int anim_clip_new_anim(struct Clip* clip, struct Node* targetNode) {
    struct Animation* newAnim = NULL;

    if (clip->numAnimations >= ((unsigned int)-1)
     || !(newAnim = realloc(clip->animations, (clip->numAnimations + 1) * sizeof(struct Animation)))) {
        fprintf(stderr, "Error: clip_new_anim: could not reallocate memory for new animation\n");
        return 0;
    }
    clip->animations = newAnim;
    newAnim = clip->animations + clip->numAnimations;
    if (!anim_animation_init(newAnim, targetNode)) {
        fprintf(stderr, "Error: clip_new_anim: could not allocate memory for track set\n");
        return 0;
    }
    clip->numAnimations++;
    return 1;
}

int anim_engine_init(struct AnimationEngine* engine) {
    engine->animQueue = NULL;
    engine->numAnimSlots = 0;
    return 1;
}

void anim_engine_free(struct AnimationEngine* engine) {
    unsigned int i;
    for (i = 0; i < engine->numAnimSlots; i++) {
        struct AnimStack* tmp;
        while ((tmp = engine->animQueue[i])) {
            engine->animQueue[i] = tmp->nextInStack;
            free(tmp);
        }
    }
    free(engine->animQueue);
}

int anim_new_slot(struct AnimationEngine* engine) {
    struct AnimStack** tmp;

    if (engine->numAnimSlots >= ((unsigned int)-1)
     || !(tmp = realloc(engine->animQueue, (engine->numAnimSlots + 1) * sizeof(struct AnimStack)))) {
        fprintf(stderr, "Error: anim_new_slot: could not reallocate memory for new anim slot\n");
        return 0;
    }
    engine->animQueue = tmp;
    engine->animQueue[engine->numAnimSlots++] = NULL;
    return 1;
}

void anim_stack_init(struct AnimStack* stack) {
    stack->clip = NULL;
    stack->delay = 0;
    stack->nextInStack = NULL;
}

int anim_stack_append(struct AnimStack* stack, struct Clip* clip, unsigned int delay) {
    struct AnimStack* newElem;

    if (!(newElem = malloc(sizeof(*newElem)))) {
        fprintf(stderr, "Error: anim_append_clip: could not allocate memory for new animStack elem\n");
        return 0;
    }

    anim_stack_init(newElem);
    newElem->clip = clip;
    newElem->delay = delay;

    while (stack->nextInStack) stack = stack->nextInStack;
    stack->nextInStack = newElem;
    return 1;
}

int anim_stack_push(struct AnimStack** stack, struct Clip* clip, unsigned int delay) {
    struct AnimStack* newElem;

    if (!(newElem = malloc(sizeof(*newElem)))) {
        fprintf(stderr, "Error: anim_append_clip: could not allocate memory for new animStack elem\n");
        return 0;
    }

    anim_stack_init(newElem);
    newElem->clip = clip;
    newElem->delay = delay;
    newElem->nextInStack = *stack;
    *stack = newElem;
    return 1;
}

void anim_stack_pop(struct AnimStack** stack) {
    struct AnimStack* tmp = *stack;
    *stack = (*stack)->nextInStack;
    if (tmp->clip->oneShot) {
        anim_clip_free(tmp->clip);
        free(tmp->clip);
    }
    free(tmp);
}

void anim_stack_flush(struct AnimStack** stack) {
    while (*stack) anim_stack_pop(stack);
}

int anim_append_clip(struct AnimationEngine* engine, struct Clip* clip, unsigned int slot, unsigned int delay) {
    struct AnimStack* newElem;
    struct AnimStack* cur;

    if (!(newElem = malloc(sizeof(*newElem)))) {
        fprintf(stderr, "Error: anim_append_clip: could not allocate memory for new animStack elem\n");
        return 0;
    }

    anim_stack_init(newElem);
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

    anim_stack_init(newElem);
    newElem->clip = clip;
    newElem->delay = delay;
    newElem->nextInStack = engine->animQueue[slot];
    engine->animQueue[slot] = newElem;
    return 1;
}
