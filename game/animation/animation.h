#ifndef ANIMATION_H
#define ANIMATION_H

#include <game/scene/node.h>
#include <game/math/linear_algebra.h>

enum TrackFlags {
    TRACKING_POS =      1 << 0,
    TRACKING_SCALE =    1 << 1,
    TRACKING_ROT =      1 << 2,
    TRACKING_QUAT =     1 << 3
};

enum TrackTargetType {
    TRACK_X_POS = 0,
    TRACK_Y_POS,
    TRACK_Z_POS,

    TRACK_X_SCALE,
    TRACK_Y_SCALE,
    TRACK_Z_SCALE,

    TRACK_X_ROT,
    TRACK_Y_ROT,
    TRACK_Z_ROT,

    TRACK_W_QUAT,
    TRACK_X_QUAT,
    TRACK_Y_QUAT,
    TRACK_Z_QUAT,

    TRACK_NB_TYPES,

    TRACK_TRANSFORM
};

struct AnimCurve {
    enum TrackCurve {
        TRACK_CURVE_NONE,
        TRACK_LINEAR,
        TRACK_BEZIER
    } curveType;
    union TrackCurveData {
        float* linear;
        Vec3* bezier;
    } values;
    char sharedValues;
};

int anim_curve_init(struct AnimCurve* curve, enum TrackCurve type, unsigned int n);
void anim_curve_free(struct AnimCurve* curve);
void anim_curve_copy(struct AnimCurve* dest, const struct AnimCurve* src);

struct Track {
    struct AnimCurve times;
    struct AnimCurve values;

    unsigned int numKeys;
    unsigned int lastIdx;
};

int anim_track_init(struct Track* track, enum TrackCurve timeCurve, enum TrackCurve valCurve, unsigned int numKeys);
void anim_track_free(struct Track* track);

struct Track* anim_new_track_set(void);

struct Animation {
    struct Node* targetNode;
    struct Track* tracks;
    enum TrackFlags flags;
};

void anim_animation_zero(struct Animation* anim);
int anim_animation_init(struct Animation* anim, struct Node* targetNode);
void anim_animation_free(struct Animation* anim);

struct Clip {
    unsigned int duration;
    enum ClipPlayMode {
        CLIP_FORWARD,
        CLIP_BACKWARD,
        CLIP_BACK_FORTH
    } mode;
    int loop;

    unsigned int numAnimations;
    struct Animation* animations;

    unsigned int curPos;
    char rev;
    char oneShot;
    char* name;
};

void anim_clip_init(struct Clip* clip);
void anim_clip_free(struct Clip* clip);
int anim_clip_new_anim(struct Clip* clip, struct Node* targetNode);
struct Clip* anim_make_clip_transition(struct Clip* target, unsigned int duration);

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
int anim_run_stack(struct AnimStack** stack, unsigned int dt);

struct AnimationEngine {
    struct AnimStack** animQueue;
    unsigned int numAnimSlots;
};

int anim_engine_init(struct AnimationEngine* engine);
void anim_engine_free(struct AnimationEngine* engine);

int anim_new_slot(struct AnimationEngine* engine);
int anim_append_clip(struct AnimationEngine* engine, struct Clip* clip, unsigned int slot, unsigned int delay);
int anim_push_clip(struct AnimationEngine* engine, struct Clip* clip, unsigned int slot, unsigned int delay);

void anim_play_track_set(struct Track* tracks, struct Node* n, enum TrackFlags flags, unsigned int curPos);
int anim_play_clip(struct Clip* clip, unsigned int dt);
void anim_run_engine(struct AnimationEngine* engine, unsigned int dt);

struct Clip* anim_make_clip_transition(struct Clip* target, unsigned int duration);

#endif
