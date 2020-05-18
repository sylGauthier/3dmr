#include "animation.h"

#ifndef TDMR_ANIMATION_PLAY_H
#define TDMR_ANIMATION_PLAY_H

void anim_play_track_set(struct Track* tracks, struct Node* n, enum TrackFlags flags, float curPos);
void anim_play(struct Animation* anim, float curPos);
int anim_play_clip(struct Clip* clip, float dt);
int anim_run_stack(struct AnimStack** stack, float dt);
void anim_run_engine(struct AnimationEngine* engine, float dt);

#endif
