#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <3dmr/animation/animation.h>
#include <3dmr/math/interp.h>

static float get_time_key(const struct Track* track, unsigned int idx) {
    switch (track->times.interp) {
        case TRACK_LINEAR: return track->times.values[idx];
        case TRACK_BEZIER: return track->times.values[3 * idx];
        default:;
    }
    return 0;
}

static int update_clip_cur_time(struct Clip* clip, float dt) {
    if (!clip->rev) {
        if (clip->curPos + dt <= clip->duration) {
            clip->curPos += dt;
            return 1;
        } else {
            if (clip->mode == CLIP_FORWARD) {
                if (clip->loop) {
                    clip->curPos = fmod(clip->curPos + dt, clip->duration);
                    return 1;
                } else {
                    clip->curPos = clip->duration;
                    return 0;
                }
            } else if (clip->mode == CLIP_BACK_FORTH) {
                clip->curPos = fmod(2.0f * clip->duration - (clip->curPos + dt), clip->duration);
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
                    clip->curPos = fmod(clip->duration + clip->curPos - dt, clip->duration);
                    return 1;
                } else if (clip->mode == CLIP_BACK_FORTH) {
                    clip->curPos = fmod(dt - clip->curPos, clip->duration);
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

static float time_linear_interp(unsigned int curFrame, float x, float* times, unsigned int nbKeys) {
    if (curFrame >= nbKeys - 1) return 1;
    if (x <= times[curFrame]) return 0;
    if (x >= times[curFrame + 1]) return 1;
    return (x - times[curFrame]) / (times[curFrame + 1] - times[curFrame]);
}

static void value_linear_interp(float* dest, unsigned int curFrame, float s, float* values, unsigned int comp, unsigned int nbKeys) {
    unsigned int i;

    if (curFrame >= nbKeys - 1) {
        memcpy(dest, &values[comp * (nbKeys - 1)], comp * sizeof(float));
    } else if (s <= 0) {
        memcpy(dest, &values[comp * curFrame], comp * sizeof(float));
    } else if (s >= 1) {
        memcpy(dest, &values[comp * (curFrame + 1)], comp * sizeof(float));
    } else {
        for (i = 0; i < comp; i++) dest[i] = values[comp * curFrame + i] + s * (values[comp * (curFrame + 1) + i] - values[comp * curFrame + i]);
    }
}

static void quat_linear_interp(float* dest, unsigned int curFrame, float s, float* values, unsigned int nbKeys) {
    if (curFrame >= nbKeys - 1) {
        memcpy(dest, &values[4 * (nbKeys - 1)], 4 * sizeof(float));
    } else if (s <= 0) {
        memcpy(dest, &values[4 * curFrame], 4 * sizeof(float));
    } else if (s >= 1) {
        memcpy(dest, &values[4 * (curFrame + 1)], 4 * sizeof(float));
    } else {
        quaternion_slerp(dest, &values[4 * curFrame], &values[4 * (curFrame + 1)], s);
    }
}

static float time_bezier_interp(unsigned int curFrame, float x, float* times, unsigned int nbKeys) {
    unsigned int a;
    float s = 0, t1, t2, c1, c2;

    if (curFrame >= nbKeys - 1) return times[3 * (nbKeys - 1)];
    if (x <= times[3 * curFrame]) return 0;
    if (x >= times[3 * (curFrame + 1)]) return 1;

    t1 = times[3 * curFrame]; t2 = times[3 * (curFrame + 1)];
    c1 = times[3 * curFrame + 2]; c2 = times[3 * (curFrame + 1) + 1];
    s = (x - t1) / (t2 - t1);

    /* Newton method to solve Bernstein polynom - OpenGEX spec page 76 */
    for (a = 0; a < 3; a++) {
        s = s - ((t2 - 3 * c2 + 3 * c1 - t1) * s * s * s + 3 * (c2 - 2 * c1 + t1) * s * s + 3 * (c1 - t1) * s + t1 - x)
              / (3 * (t2 - 3 * c2 + 3 * c1 - t1) * s * s + 6 * (c2 - 2 * c1 + t1) * s + 3 * (c1 - t1));
    }
    return s;
}

static float bern_pol(float s, float v1, float v2, float p1, float p2) {
    return (1 - s) * (1 - s) * (1 - s) * v1 + 3 * s * (1 - s) * (1 - s) * p1 + 3 * s * s * (1 - s) * p2 + s * s * s * v2;
}

static void value_bezier_interp(float* dest, unsigned int curFrame, float s, float* points, unsigned int comp, unsigned int nbKeys) {
    unsigned int i;

    if (curFrame >= nbKeys - 1) {
        memcpy(dest, &points[3 * comp * (nbKeys - 1)], comp * sizeof(float));
    } else if (s <= 0) {
        memcpy(dest, &points[3 * comp * curFrame], comp * sizeof(float));
    } else if (s >= 1) {
        memcpy(dest, &points[3 * comp * (curFrame + 1)], comp * sizeof(float));
    } else {
        for (i = 0; i < comp; i++) {
            dest[i] = bern_pol(s, points[3 * comp * curFrame + i],            points[3 * comp * (curFrame + 1) + i],
                                  points[3 * comp * curFrame + 2 * comp + i], points[3 * comp * (curFrame + 1) + comp + i]);
        }
    }
}

static unsigned int get_cur_frame(struct Track* track, float x) {
    unsigned int a;

    if (x <= get_time_key(track, 0)) return 0;
    if (x >= get_time_key(track, track->numKeys - 1)) return track->numKeys - 1;
    if (track->lastIdx >= track->numKeys - 1) {
        track->lastIdx = 0;
    }
    for (a = track->lastIdx; !(get_time_key(track, a) <= x && x <= get_time_key(track, a + 1)); a = (a + 1) % track->numKeys);
    track->lastIdx = a;
    return a;
}

static void interp_track(struct Track* track, float time, float* dest) {
    float s;
    unsigned int curFrame;

    curFrame = get_cur_frame(track, time);

    switch (track->times.interp) {
        case TRACK_LINEAR:
            s = time_linear_interp(curFrame, time, track->times.values, track->numKeys);
            break;
        case TRACK_BEZIER:
            s = time_bezier_interp(curFrame, time, track->times.values, track->numKeys);
            break;
        default:
            s = 0.;
    }
    switch (track->values.interp) {
        case TRACK_LINEAR:
            value_linear_interp(dest, curFrame, s, track->values.values, track->values.numComponent, track->numKeys);
            break;
        case TRACK_BEZIER:
            value_bezier_interp(dest, curFrame, s, track->values.values, track->values.numComponent, track->numKeys);
            break;
        default:
            break;
    }
}

static void interp_quat_track(struct Track* track, float time, float* dest) {
    float s;
    unsigned int curFrame;

    curFrame = get_cur_frame(track, time);

    switch (track->times.interp) {
        case TRACK_LINEAR:
            s = time_linear_interp(curFrame, time, track->times.values, track->numKeys);
            break;
        case TRACK_BEZIER:
            s = time_bezier_interp(curFrame, time, track->times.values, track->numKeys);
            break;
        default:
            s = 0.;
    }
    switch (track->values.interp) {
        case TRACK_LINEAR:
            quat_linear_interp(dest, curFrame, s, track->values.values, track->numKeys);
            break;
        default:
            break;
    }
}

void anim_track_play(struct Node* n, struct Track* track, float curPos) {
    Vec3 rot = {0};

    switch (track->channel) {
        case TRACK_POS:
            interp_track(track, curPos, n->position);
            n->changedFlags |= POSITION_CHANGED;
            break;
        case TRACK_SCALE:
            interp_track(track, curPos, n->scale);
            n->changedFlags |= SCALE_CHANGED;
            break;
        case TRACK_ROT:
            interp_track(track, curPos, rot);
            quaternion_from_xyz(n->orientation, rot);
            n->changedFlags |= ORIENTATION_CHANGED;
            break;
        case TRACK_QUAT:
            interp_quat_track(track, curPos, n->orientation);
            n->changedFlags |= ORIENTATION_CHANGED;
            break;
        case TRACK_X_POS:
        case TRACK_Y_POS:
        case TRACK_Z_POS:
            interp_track(track, curPos, n->position + track->channel - TRACK_X_POS);
            n->changedFlags |= POSITION_CHANGED;
            break;
        case TRACK_X_SCALE:
        case TRACK_Y_SCALE:
        case TRACK_Z_SCALE:
            interp_track(track, curPos, n->scale + track->channel - TRACK_X_SCALE);
            n->changedFlags |= SCALE_CHANGED;
            break;
        case TRACK_X_ROT:
        case TRACK_Y_ROT:
        case TRACK_Z_ROT:
            quaternion_to_xyz(rot, n->orientation);
            interp_track(track, curPos, rot + track->channel - TRACK_X_ROT);
            quaternion_from_xyz(n->orientation, rot);
            n->changedFlags |= ORIENTATION_CHANGED;
            break;
        default:
            break;
    }
}

void anim_play(struct Animation* anim, float curPos) {
    unsigned int i;

    for (i = 0; i < anim->numTracks; i++) {
        anim_track_play(anim->targetNode, &anim->tracks[i], curPos);
    }
}

int anim_play_clip(struct Clip* clip, float dt) {
    unsigned int i;
    int running;

    if (!clip->duration) return 1;

    running = update_clip_cur_time(clip, dt);
    for (i = 0; i < clip->numAnimations; i++) {
        anim_play(&clip->animations[i], clip->curPos);
    }
    return running;
}

int anim_run_stack(struct AnimStack** stack, float dt) {
    struct AnimStack* cur = *stack;

    if (cur && cur->delay < dt) {
        cur->delay = 0;
        if (!anim_play_clip(cur->clip, dt)) {
            anim_stack_pop(stack);
        }
    } else if (cur) {
        cur->delay -= dt;
    } else {
        return 0;
    }
    return 1;
}

void anim_run_engine(struct AnimationEngine* engine, float dt) {
    unsigned int i;

    for (i = 0; i < engine->numAnimSlots; i++) {
        anim_run_stack(engine->animQueue + i, dt);
    }
}
