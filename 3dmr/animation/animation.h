#ifndef TDMR_ANIMATION_H
#define TDMR_ANIMATION_H

#include <3dmr/scene/node.h>
#include <3dmr/math/linear_algebra.h>

enum TrackChannel {
    /* Pos, Scale and Rot channels can be split into separate components */
    TRACK_X_POS = 0,
    TRACK_Y_POS,
    TRACK_Z_POS,
    TRACK_POS,

    TRACK_X_SCALE,
    TRACK_Y_SCALE,
    TRACK_Z_SCALE,
    TRACK_SCALE,

    TRACK_X_ROT,
    TRACK_Y_ROT,
    TRACK_Z_ROT,
    TRACK_ROT,

    /* Quat has to be animated as a whole (doesn't really make sense to animate single components) */
    TRACK_QUAT,

    TRACK_TRANSFORM,

    TRACK_NB_TYPES
};

struct AnimCurve {
    enum TrackInterp {
        TRACK_INTERP_NONE,
        TRACK_LINEAR,
        TRACK_BEZIER,
        TRACK_CUBIC_SPLINE
    } interp;
    unsigned int numComponent;
    float* values;
    char sharedValues;
};

int anim_curve_init(struct AnimCurve* curve, enum TrackInterp interp, unsigned int comp, unsigned int n);
void anim_curve_free(struct AnimCurve* curve);
void anim_curve_copy(struct AnimCurve* dest, const struct AnimCurve* src);

struct Track {
    struct AnimCurve times;
    struct AnimCurve values;

    unsigned int numKeys;
    unsigned int lastIdx;

    enum TrackChannel channel;
};

int anim_track_init(struct Track* track, enum TrackChannel channel, enum TrackInterp timeInterp, enum TrackInterp valInterp, unsigned int numKeys);
void anim_track_free(struct Track* track);

struct Animation {
    struct Node* targetNode;
    struct Track* tracks;
    unsigned int numTracks;
};

int anim_animation_init(struct Animation* anim, struct Node* targetNode);
int anim_animation_new_track(struct Animation* anim, enum TrackChannel channel,
                             enum TrackInterp timeInterp, enum TrackInterp valInterp,
                             unsigned int numKeys, unsigned int* trackIdx);
void anim_animation_free(struct Animation* anim);

struct Clip {
    float duration;
    enum ClipPlayMode {
        CLIP_FORWARD,
        CLIP_BACKWARD,
        CLIP_BACK_FORTH
    } mode;
    int loop;

    unsigned int numAnimations;
    struct Animation* animations;

    float curPos;
    char rev;
    char oneShot;
    char* name;
};

void anim_clip_init(struct Clip* clip);
void anim_clip_free(struct Clip* clip);
int anim_clip_new_anim(struct Clip* clip, struct Node* targetNode);
struct AnimStack {
    struct Clip* clip;
    unsigned int delay;

    struct AnimStack* nextInStack;
};

void anim_stack_init(struct AnimStack* stack);
int anim_stack_append(struct AnimStack* stack, struct Clip* clip, unsigned int delay);
int anim_stack_push(struct AnimStack** stack, struct Clip* clip, unsigned int delay);
void anim_stack_pop(struct AnimStack** stack);
void anim_stack_flush(struct AnimStack** stack);

struct AnimationEngine {
    struct AnimStack** animQueue;
    unsigned int numAnimSlots;
};

int anim_engine_init(struct AnimationEngine* engine);
void anim_engine_free(struct AnimationEngine* engine);

int anim_new_slot(struct AnimationEngine* engine);
int anim_append_clip(struct AnimationEngine* engine, struct Clip* clip, unsigned int slot, unsigned int delay);
int anim_push_clip(struct AnimationEngine* engine, struct Clip* clip, unsigned int slot, unsigned int delay);

#endif
