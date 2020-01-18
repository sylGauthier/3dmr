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
        TRACK_LINEAR,
        TRACK_BEZIER
    } curveType;
    void* values;
    char sharedValues;
};

struct Track {
    struct AnimCurve times;
    struct AnimCurve values;

    unsigned int nbKeys;
    unsigned int lastIdx;
};

struct Animation {
    struct Node* targetNode;
    struct Track* tracks;
    char flags;
};

enum ClipPlayMode {
    CLIP_FORWARD,
    CLIP_BACKWARD,
    CLIP_BACK_FORTH
};

struct Clip {
    unsigned int duration;
    char mode, loop;

    unsigned int nbAnimations;
    struct Animation* animations;

    unsigned int curPos;
    char rev;
};

struct AnimStack {
    struct Clip* clip;
    unsigned int delay;

    struct AnimStack* nextInStack;
};

struct AnimationEngine {
    struct AnimStack** animQueue;
    unsigned int nbAnimSlots;
};

int anim_track_init(struct Track* track, enum TrackCurve timeCurve,
                    enum TrackCurve valCurve, unsigned int nbKeys);
struct Track* anim_new_track_set();

void anim_track_pos(struct Animation* anim);
void anim_track_scale(struct Animation* anim);
void anim_track_rot(struct Animation* anim);
void anim_track_quat(struct Animation* anim);
void anim_track_transform(struct Animation* anim);

void anim_clip_init(struct Clip* clip);
int anim_clip_new_anim(struct Clip* clip, struct Node* targetNode);

int anim_new_slot(struct AnimationEngine* engine);
int anim_append_clip(struct AnimationEngine* engine, struct Clip* clip, unsigned int slot, unsigned int delay);
int anim_push_clip(struct AnimationEngine* engine, struct Clip* clip, unsigned int slot, unsigned int delay);

void anim_play_track_set(struct Track* tracks, struct Node* n, char flags, unsigned int curPos);
int anim_play_clip(struct Clip* clip, unsigned int dt);
void anim_run_engine(struct AnimationEngine* engine, unsigned int dt);

void anim_free_track(struct Track* track);
void anim_free_animation(struct Animation* anim);
void anim_free_clip(struct Clip* clip);
void anim_free_engine(struct AnimationEngine* engine);

#endif
