#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <game/animation/animation.h>

int anim_track_init(struct Track* track, enum TrackCurve timeCurve, enum TrackCurve valCurve, unsigned int nbKeys) {
    unsigned int timeSize = nbKeys * (timeCurve == TRACK_LINEAR ? sizeof(float) : sizeof(Vec3));
    unsigned int valSize = nbKeys * (valCurve == TRACK_LINEAR ? sizeof(float) : sizeof(Vec3));
    track->times.values = NULL;
    track->values.values = NULL;
    track->times.curveType = timeCurve;
    track->values.curveType = valCurve;
    track->times.sharedValues = 0;
    track->values.sharedValues = 0;
    if (!(track->times.values = malloc(timeSize)) || !(track->values.values = malloc(valSize))) {
        fprintf(stderr, "Error: anim_track_init: could not allocate memory for keys\n");
        free(track->times.values);
        return 0;
    }
    track->nbKeys = nbKeys;
    track->lastIdx = 0;
    return 1;
}

void anim_clip_init(struct Clip* clip) {
    clip->duration = 0;
    clip->mode = CLIP_FORWARD;
    clip->loop = 0;

    clip->nbAnimations = 0;
    clip->animations = NULL;

    clip->curPos = 0;
    clip->rev = 0;
    clip->name = NULL;
}

struct Track* anim_new_track_set() {
    return calloc(TRACK_NB_TYPES, sizeof(struct Track));
}

void anim_track_pos(struct Animation* anim) {
    anim->flags |= TRACKING_POS;
}

void anim_track_scale(struct Animation* anim) {
    anim->flags |= TRACKING_SCALE;
}

void anim_track_rot(struct Animation* anim) {
    anim->flags |= TRACKING_ROT;
}

void anim_track_quat(struct Animation* anim) {
    anim->flags |= TRACKING_QUAT;
}

void anim_track_transform(struct Animation* anim) {
    anim_track_pos(anim);
    anim_track_scale(anim);
    anim_track_quat(anim);
}

int anim_clip_new_anim(struct Clip* clip, struct Node* targetNode) {
    void* tmp;
    struct Animation* newAnim = NULL;

    if (!(tmp = realloc(clip->animations, (clip->nbAnimations + 1) * sizeof(struct Animation)))) {
        fprintf(stderr, "Error: clip_new_anim: could not reallocate memory for new animation\n");
        return -1;
    }
    clip->animations = tmp;
    newAnim = clip->animations + clip->nbAnimations;
    newAnim->targetNode = targetNode;
    if (!(newAnim->tracks = anim_new_track_set())) {
        fprintf(stderr, "Error: clip_new_anim: could not allocate memory for track set\n");
        return -1;
    }
    newAnim->flags = 0;
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

void anim_free_track(struct Track* track) {
    if (!track->times.sharedValues) free(track->times.values);
    if (!track->values.sharedValues) free(track->values.values);
}

void anim_free_animation(struct Animation* anim) {
    if (anim->tracks) {
        unsigned int i;
        for (i = 0; i < TRACK_NB_TYPES; i++) {
            anim_free_track(&anim->tracks[i]);
        }
        free(anim->tracks);
    }
}

void anim_free_clip(struct Clip* clip) {
    unsigned int i;
    for (i = 0; i < clip->nbAnimations; i++) {
        anim_free_animation(clip->animations + i);
    }
    free(clip->animations);
    free(clip->name);
}

void anim_free_engine(struct AnimationEngine* engine) {
    unsigned int i;
    for (i = 0; i < engine->nbAnimSlots; i++) {
        struct AnimStack* tmp;
        while ((tmp = engine->animQueue[i])) {
            engine->animQueue[i] = tmp->nextInStack;
            free(tmp);
        }
    }
    free(engine->animQueue);
}
