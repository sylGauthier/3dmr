#include <stdlib.h>
#include <stdio.h>

#include <game/animation/animation.h>

#define GET_TIME_KEY(track, idx) ((track)->times.curveType == TRACK_LINEAR ? ((float*)(track)->times.values)[(idx)] \
                                                                           : ((Vec3*)(track)->times.values)[(idx)][0])

static int update_clip_cur_time(struct Clip* clip, unsigned int dt) {
    if (!clip->rev) {
        if (clip->curPos + dt <= clip->duration) {
            clip->curPos += dt;
            return 1;
        } else {
            if (clip->mode == CLIP_FORWARD) {
                if (clip->loop) {
                    clip->curPos = (clip->curPos + dt) % clip->duration;
                    return 1;
                } else {
                    clip->curPos = clip->duration;
                    return 0;
                }
            } else if (clip->mode == CLIP_BACK_FORTH) {
                clip->curPos = (2 * clip->duration - (clip->curPos + dt)) % clip->duration;
                clip->rev = 1;
                return 1;
            }
        }
    } else {
        if (clip->curPos > dt) {
            clip->curPos -= dt;
            return 1;
        } else {
            if (clip->loop) {
                if (clip->mode == CLIP_BACKWARD) {
                    clip->curPos = (clip->duration + clip->curPos - dt) % clip->duration;
                    return 1;
                } else if (clip->mode == CLIP_BACK_FORTH) {
                    clip->curPos = (dt - clip->curPos) % clip->duration;
                    clip->rev = 0;
                    return 1;
                }
            } else {
                clip->curPos = 0;
                return 0;
            }
        }
    }
    return 0;
}

static float time_linear_interp(int curFrame, float x, float* times, unsigned int nbKeys) {
    if (curFrame >= nbKeys-1) return 1;
    if (x <= times[curFrame]) return 0;
    if (x >= times[curFrame + 1]) return 1;
    return (x - times[curFrame]) / (times[curFrame + 1] - times[curFrame]);
}

static float value_linear_interp(int curFrame, float s, float* values, unsigned int nbKeys) {
    if (curFrame >= nbKeys-1) return values[nbKeys - 1];
    if (s <= 0) return values[curFrame];
    if (s >= 1) return values[curFrame + 1];
    return (1 - s) * values[curFrame] + s * values[curFrame + 1];
}

static float time_bezier_interp(int curFrame, float x, Vec3* times, unsigned int nbKeys) {
    unsigned int a;
    float s = 0, t1, t2, c1, c2;
    float remain;

    if (curFrame >= nbKeys-1) return times[nbKeys - 1][0];
    if (x <= times[curFrame][0]) return 0;
    if (x >= times[curFrame + 1][0]) return 1;

    t1 = times[curFrame][0]; t2 = times[curFrame + 1][0];
    c1 = times[curFrame][2]; c2 = times[curFrame + 1][1];
    s = (x - t1) / (t2 - t1);

    /* Newton method to solve Bernstein polynom - OpenGEX spec page 76 */
    for (a = 0; a < 3; a++) {
        s = s - ((t2 - 3 * c2 + 3 * c1 - t1) * s * s * s + 3 * (c2 - 2 * c1 + t1) * s * s + 3 * (c1 - t1) * s + t1 - x)
              / (3 * (t2 - 3 * c2 + 3 * c1 - t1) * s * s + 6 * (c2 - 2 * c1 + t1) * s + 3 * (c1 - t1));
    }
    remain = (1-s)*(1-s)*(1-s)*t1 + 3*s*(1-s)*(1-s)*c1 + 3*s*s*(1-s)*c2 + s*s*s*t2 - x;
    return s;
}

static float value_bezier_interp(int curFrame, float s, Vec3* points, unsigned int nbKeys) {
    float v1, v2, p1, p2;

    if (curFrame >= nbKeys-1) return points[nbKeys - 1][0];
    if (s <= 0) return points[curFrame][0];
    if (s >= 1) return points[curFrame + 1][0];

    v1 = points[curFrame][0]; v2 = points[curFrame + 1][0];
    p1 = points[curFrame][2]; p2 = points[curFrame + 1][1];

    return (1 - s) * (1 - s) * (1 - s) * v1 + 3 * s * (1 - s) * (1 - s) * p1 + 3 * s * s * (1 - s) * p2 + s * s * s * v2;
}

static int get_cur_frame(struct Track* track, unsigned int curPos) {
    unsigned int a;
    float x = (float)curPos / 1000;

    if (x <= GET_TIME_KEY(track, 0)) return 0;
    if (x >= GET_TIME_KEY(track, track->nbKeys - 1)) return track->nbKeys - 1;
    if (track->lastIdx >= track->nbKeys - 1) {
        track->lastIdx = 0;
    }
    for (a = track->lastIdx; !(GET_TIME_KEY(track, a) <= x && x <= GET_TIME_KEY(track, a + 1)); a = (a + 1) % track->nbKeys);
    track->lastIdx = a;
    return a;
}

static float interp_track(struct Track* track, unsigned int curPos) {
    float time = (float)curPos / 1000.0;
    float s;
    int curFrame;

    curFrame = get_cur_frame(track, curPos);

    switch (track->times.curveType) {
        case TRACK_LINEAR:
            s = time_linear_interp(curFrame, time, track->times.values, track->nbKeys);
            break;
        case TRACK_BEZIER:
            s = time_bezier_interp(curFrame, time, track->times.values, track->nbKeys);
            break;
        default:
            s = 0.;
    }
    switch (track->values.curveType) {
        case TRACK_LINEAR:
            return value_linear_interp(curFrame, s, track->values.values, track->nbKeys);
        case TRACK_BEZIER:
            return value_bezier_interp(curFrame, s, track->values.values, track->nbKeys);
        default:
            return 0.;
    }
}

static void play_animation(struct Animation* anim, unsigned int curPos) {
    char rotTracked = 0;
    struct Node* n = anim->targetNode;
    Vec3 rot;

    if (anim->tracks[TRACK_X_POS].nbKeys) {
        n->position[0] = interp_track(&anim->tracks[TRACK_X_POS], curPos);
        n->changedFlags |= POSITION_CHANGED;
    }
    if (anim->tracks[TRACK_Y_POS].nbKeys) {
        n->position[1] = interp_track(&anim->tracks[TRACK_Y_POS], curPos);
        n->changedFlags |= POSITION_CHANGED;
    }
    if (anim->tracks[TRACK_Z_POS].nbKeys) {
        n->position[2] = interp_track(&anim->tracks[TRACK_Z_POS], curPos);
        n->changedFlags |= POSITION_CHANGED;
    }
    if (anim->tracks[TRACK_X_ROT].nbKeys) {
        rot[0] = interp_track(&anim->tracks[TRACK_X_ROT], curPos);
        rotTracked = 1;
    }
    if (anim->tracks[TRACK_Y_ROT].nbKeys) {
        rot[1] = interp_track(&anim->tracks[TRACK_Y_ROT], curPos);
        rotTracked = 1;
    }
    if (anim->tracks[TRACK_Z_ROT].nbKeys) {
        rot[2] = interp_track(&anim->tracks[TRACK_Z_ROT], curPos);
        rotTracked = 1;
    }
    if (anim->tracks[TRACK_X_SCALE].nbKeys) {
        n->scale[0] = interp_track(&anim->tracks[TRACK_X_SCALE], curPos);
        n->changedFlags |= SCALE_CHANGED;
    }
    if (anim->tracks[TRACK_Y_SCALE].nbKeys) {
        n->scale[1] = interp_track(&anim->tracks[TRACK_Y_SCALE], curPos);
        n->changedFlags |= SCALE_CHANGED;
    }
    if (anim->tracks[TRACK_Z_SCALE].nbKeys) {
        n->scale[2] = interp_track(&anim->tracks[TRACK_Z_SCALE], curPos);
        n->changedFlags |= SCALE_CHANGED;
    }
    if (rotTracked) {
        quaternion_from_xyz(n->orientation, rot);
        n->changedFlags |= ORIENTATION_CHANGED;
    }
}

static int play_clip(struct Clip* clip, unsigned int dt) {
    unsigned int i;
    int running;

    running = update_clip_cur_time(clip, dt);

    for (i = 0; i < clip->nbAnimations; i++) {
        play_animation(&clip->animations[i], clip->curPos);
    }
    return running;
}

static void pop_anim_stack(struct AnimationEngine* engine, unsigned int slot) {
    void* tmp = engine->animQueue[slot];
    engine->animQueue[slot] = engine->animQueue[slot]->nextInStack;
    free(tmp);
}

void anim_play_all(struct AnimationEngine* engine, unsigned int dt) {
    unsigned int i;

    for (i = 0; i < engine->nbAnimSlots; i++) {
        struct AnimStack* cur = engine->animQueue[i];

        if (cur && cur->delay < dt) {
            cur->delay = 0;
            if (!play_clip(cur->clip, dt)) {
                pop_anim_stack(engine, i);
            }
        } else if (cur) {
            cur->delay -= dt;
        }
    }
}
