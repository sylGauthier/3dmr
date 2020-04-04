#include <stdlib.h>
#include <stdio.h>

#include <game/animation/animation.h>

static float get_time_key(const struct Track* track, unsigned int idx) {
    switch (track->times.curveType) {
        case TRACK_LINEAR: return track->times.values.linear[idx];
        case TRACK_BEZIER: return track->times.values.bezier[idx][0];
        default:;
    }
    return 0;
}

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

    if (x <= get_time_key(track, 0)) return 0;
    if (x >= get_time_key(track, track->numKeys - 1)) return track->numKeys - 1;
    if (track->lastIdx >= track->numKeys - 1) {
        track->lastIdx = 0;
    }
    for (a = track->lastIdx; !(get_time_key(track, a) <= x && x <= get_time_key(track, a + 1)); a = (a + 1) % track->numKeys);
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
            s = time_linear_interp(curFrame, time, track->times.values.linear, track->numKeys);
            break;
        case TRACK_BEZIER:
            s = time_bezier_interp(curFrame, time, track->times.values.bezier, track->numKeys);
            break;
        default:
            s = 0.;
    }
    switch (track->values.curveType) {
        case TRACK_LINEAR:
            return value_linear_interp(curFrame, s, track->values.values.linear, track->numKeys);
        case TRACK_BEZIER:
            return value_bezier_interp(curFrame, s, track->values.values.bezier, track->numKeys);
        default:
            return 0.;
    }
}

void anim_play_track_set(struct Track* tracks, struct Node* n, enum TrackFlags flags, unsigned int curPos) {
    Vec3 rot;

    if (flags & TRACKING_POS) {
        if (tracks[TRACK_X_POS].numKeys) {
            n->position[0] = interp_track(&tracks[TRACK_X_POS], curPos);
        }
        if (tracks[TRACK_Y_POS].numKeys) {
            n->position[1] = interp_track(&tracks[TRACK_Y_POS], curPos);
        }
        if (tracks[TRACK_Z_POS].numKeys) {
            n->position[2] = interp_track(&tracks[TRACK_Z_POS], curPos);
        }
        n->changedFlags |= POSITION_CHANGED;
    }
    if (flags & TRACKING_SCALE) {
        if (tracks[TRACK_X_SCALE].numKeys) {
            n->scale[0] = interp_track(&tracks[TRACK_X_SCALE], curPos);
        }
        if (tracks[TRACK_Y_SCALE].numKeys) {
            n->scale[1] = interp_track(&tracks[TRACK_Y_SCALE], curPos);
        }
        if (tracks[TRACK_Z_SCALE].numKeys) {
            n->scale[2] = interp_track(&tracks[TRACK_Z_SCALE], curPos);
        }
        n->changedFlags |= SCALE_CHANGED;
    }
    if (flags & TRACKING_ROT) {
        if (tracks[TRACK_X_ROT].numKeys) {
            rot[0] = interp_track(&tracks[TRACK_X_ROT], curPos);
        }
        if (tracks[TRACK_Y_ROT].numKeys) {
            rot[1] = interp_track(&tracks[TRACK_Y_ROT], curPos);
        }
        if (tracks[TRACK_Z_ROT].numKeys) {
            rot[2] = interp_track(&tracks[TRACK_Z_ROT], curPos);
        }
        quaternion_from_xyz(n->orientation, rot);
        n->changedFlags |= ORIENTATION_CHANGED;
    }
    if (flags & TRACKING_QUAT) {
        if (tracks[TRACK_W_QUAT].numKeys) {
            n->orientation[0] = interp_track(&tracks[TRACK_W_QUAT], curPos);
        }
        if (tracks[TRACK_X_QUAT].numKeys) {
            n->orientation[1] = interp_track(&tracks[TRACK_X_QUAT], curPos);
        }
        if (tracks[TRACK_Y_QUAT].numKeys) {
            n->orientation[2] = interp_track(&tracks[TRACK_Y_QUAT], curPos);
        }
        if (tracks[TRACK_Z_QUAT].numKeys) {
            n->orientation[3] = interp_track(&tracks[TRACK_Z_QUAT], curPos);
        }
        n->changedFlags |= ORIENTATION_CHANGED;
    }
}

int anim_play_clip(struct Clip* clip, unsigned int dt) {
    unsigned int i;
    int running;

    if (!clip->duration) return 1;

    running = update_clip_cur_time(clip, dt);
    for (i = 0; i < clip->numAnimations; i++) {
        anim_play_track_set(clip->animations[i].tracks, clip->animations[i].targetNode,
                            clip->animations[i].flags, clip->curPos);
    }
    return running;
}

int anim_run_stack(struct AnimStack** stack, unsigned int dt) {
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

void anim_run_engine(struct AnimationEngine* engine, unsigned int dt) {
    unsigned int i;

    for (i = 0; i < engine->numAnimSlots; i++) {
        anim_run_stack(engine->animQueue + i, dt);
    }
}
