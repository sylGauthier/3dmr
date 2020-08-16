#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <3dmr/animation/animation.h>

int anim_curve_init(struct AnimCurve* curve, enum TrackInterp interp, unsigned int comp, unsigned int n) {
    curve->sharedValues = 0;
    curve->numComponent = comp;
    curve->values = NULL;
    switch (curve->interp = interp) {
        case TRACK_INTERP_NONE: return 1;
        case TRACK_LINEAR: return n < ((unsigned int)-1) / (sizeof(float) * comp) && (curve->values = malloc(comp * n * sizeof(float)));
        case TRACK_BEZIER: return n < ((unsigned int)-1) / (sizeof(Vec3) * comp) && (curve->values = malloc(comp * n * sizeof(Vec3)));
        case TRACK_CUBIC_SPLINE:
            fprintf(stderr, "Error: anim_curve_init: cubic spline not implemented yet\n");
            return 0;
    }
    return 0;
}

void anim_curve_free(struct AnimCurve* curve) {
    if (curve->sharedValues) return;
    free(curve->values);
}

void anim_curve_copy(struct AnimCurve* dest, const struct AnimCurve* src) {
    *dest = *src;
    dest->sharedValues = 1;
}

int anim_track_init(struct Track* track, enum TrackChannel channel, enum TrackInterp timeInterp, enum TrackInterp valInterp, unsigned int numKeys) {
    int ok;
    if (!anim_curve_init(&track->times, timeInterp, 1, numKeys)) return 0;

    switch (channel) {
        case TRACK_X_POS:
        case TRACK_Y_POS:
        case TRACK_Z_POS:
        case TRACK_X_SCALE:
        case TRACK_Y_SCALE:
        case TRACK_Z_SCALE:
        case TRACK_X_ROT:
        case TRACK_Y_ROT:
        case TRACK_Z_ROT:
            ok = anim_curve_init(&track->values, valInterp, 1, numKeys);
            break;
        case TRACK_POS:
        case TRACK_SCALE:
        case TRACK_ROT:
            ok = anim_curve_init(&track->values, valInterp, 3, numKeys);
            break;
        case TRACK_QUAT:
            ok = anim_curve_init(&track->values, valInterp, 4, numKeys);
            break;
        default:
            ok = 0;
            break;
    }
    if (!ok) {
        anim_curve_free(&track->times);
        return 0;
    }
    track->channel = channel;
    track->numKeys = numKeys;
    track->lastIdx = 0;
    return 1;
}

void anim_track_free(struct Track* track) {
    anim_curve_free(&track->times);
    anim_curve_free(&track->values);
}

int anim_animation_init(struct Animation* anim, struct Node* targetNode) {
    anim->tracks = 0;
    anim->numTracks = 0;
    anim->targetNode = targetNode;
    return 1;
}

int anim_animation_new_track(struct Animation* anim, enum TrackChannel channel,
                             enum TrackInterp timeInterp, enum TrackInterp valInterp,
                             unsigned int numKeys, unsigned int* trackIdx) {
    unsigned int i;
    void* tmp;

    if (!numKeys) return 0;

    for (i = 0; i < anim->numTracks; i++) {
        enum TrackChannel cur = anim->tracks[i].channel;
        if (       cur == channel
                || (cur == TRACK_POS && (channel == TRACK_X_POS || channel == TRACK_Y_POS || channel == TRACK_Z_POS))
                || (cur == TRACK_SCALE && (channel == TRACK_X_SCALE || channel == TRACK_Y_SCALE || channel == TRACK_Z_SCALE))
                || (cur == TRACK_ROT && (channel == TRACK_X_ROT || channel == TRACK_Y_ROT || channel == TRACK_Z_ROT))
                || (channel == TRACK_POS && (cur == TRACK_X_POS || cur == TRACK_Y_POS || cur == TRACK_Z_POS))
                || (channel == TRACK_SCALE && (cur == TRACK_X_SCALE || cur == TRACK_Y_SCALE || cur == TRACK_Z_SCALE))
                || (channel == TRACK_ROT && (cur == TRACK_X_ROT || cur == TRACK_Y_ROT || cur == TRACK_Z_ROT))) {
            fprintf(stderr, "Error: anim_animation_new_track: redundant anim channel\n");
            return 0;
        }
    }
    if (!(tmp = realloc(anim->tracks, (anim->numTracks + 1) * sizeof(*anim->tracks)))) {
        fprintf(stderr, "Error: anim_animation_new_track: can't allocate memory for new track\n");
        return 0;
    }
    anim->tracks = tmp;
    *trackIdx = anim->numTracks++;
    return anim_track_init(&anim->tracks[anim->numTracks - 1], channel, timeInterp, valInterp, numKeys);
}

void anim_animation_free(struct Animation* anim) {
    if (anim->tracks) {
        unsigned int i;
        for (i = 0; i < anim->numTracks; i++) {
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
     || !(tmp = realloc(engine->animQueue, (engine->numAnimSlots + 1) * sizeof(*engine->animQueue)))) {
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
