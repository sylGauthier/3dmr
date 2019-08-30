#ifndef ANIMATION_H
#define ANIMATION_H

#include <game/scene/node.h>
#include <game/math/linear_algebra.h>

enum TrackTargetType {
    TRACK_X_POS = 0,
    TRACK_Y_POS,
    TRACK_Z_POS,
    TRACK_X_ROT,
    TRACK_Y_ROT,
    TRACK_Z_ROT,
    TRACK_X_SCALE,
    TRACK_Y_SCALE,
    TRACK_Z_SCALE,
    TRACK_NB_TYPES
};

struct AnimCurve {
    enum TrackCurve {
        TRACK_LINEAR,
        TRACK_BEZIER
    } curveType;
    void* values;
};

struct Track {
    struct AnimCurve times;
    struct AnimCurve values;

    unsigned int nbKeys;
    unsigned int lastIdx;
};

struct Animation {
    struct Node* targetNode;

    struct Track tracks[TRACK_NB_TYPES];
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

void clip_init(struct Clip* clip);
int clip_new_anim(struct Clip* clip, struct Node* targetNode);

int anim_new_slot(struct AnimationEngine* engine);
int anim_append_clip(struct AnimationEngine* engine, struct Clip* clip, unsigned int slot, unsigned int delay);
int anim_push_clip(struct AnimationEngine* engine, struct Clip* clip, unsigned int slot, unsigned int delay);

void anim_play_all(struct AnimationEngine* engine, unsigned int dt);

#endif
