#include <stdlib.h>
#include <string.h>

#include <3dmr/animation/animation.h>

static void make_curve_transition(struct AnimCurve* dest, const struct AnimCurve* targetCurve, struct Node* targetNode, enum TrackChannel t) {
    Vec3 eulerRot;
    float *destf = dest->values;

    switch (t) {
        case TRACK_X_POS:
        case TRACK_Y_POS:
        case TRACK_Z_POS:
            destf[0] = targetNode->position[t - TRACK_X_POS];
            break;
        case TRACK_POS:
            memcpy(destf, targetNode->position, sizeof(Vec3));
            break;
        case TRACK_X_SCALE:
        case TRACK_Y_SCALE:
        case TRACK_Z_SCALE:
            destf[0] = targetNode->scale[t - TRACK_X_SCALE];
            break;
        case TRACK_SCALE:
            memcpy(destf, targetNode->scale, sizeof(Vec3));
            break;
        case TRACK_X_ROT:
        case TRACK_Y_ROT:
        case TRACK_Z_ROT:
            quaternion_to_xyz(eulerRot, targetNode->orientation);
            destf[0] = eulerRot[t - TRACK_X_ROT];
            break;
        case TRACK_ROT:
            quaternion_to_xyz(destf, targetNode->orientation);
            break;
        case TRACK_QUAT:
            memcpy(destf, targetNode->orientation, sizeof(Vec4));
            break;
        default:
            break;
    }
    switch (targetCurve->interp) {
        case TRACK_LINEAR:
            memcpy(destf + dest->numComponent, targetCurve->values, dest->numComponent * sizeof(float));
            break;
        case TRACK_BEZIER:
            memcpy(destf + 3 * dest->numComponent, targetCurve->values, 3 * dest->numComponent * sizeof(float));
            break;
        default:
            break;
    }
}

static int make_anim_transition(struct Animation* transition, const struct Animation* target, float duration) {
    unsigned int i, f;

    if (!anim_animation_init(transition, target->targetNode)) return 0;
    if (!target->numTracks) return 1;
    if (!anim_animation_new_track(transition, target->tracks[0].channel, TRACK_LINEAR, TRACK_LINEAR, 2, &f)) return 0;
    transition->tracks[0].times.values[0] = 0;
    transition->tracks[0].times.values[1] = duration;

    make_curve_transition(&transition->tracks[0].values, &target->tracks[0].values, target->targetNode, target->tracks[0].channel);
    for (i = 1; i < target->numTracks; i++) {
        if (!anim_animation_new_track(transition, target->tracks[i].channel, TRACK_INTERP_NONE, TRACK_LINEAR, 2, &f)) {
            anim_animation_free(transition);
            return 0;
        }
        anim_curve_copy(&transition->tracks[i].times, &transition->tracks[0].times);
        make_curve_transition(&transition->tracks[i].values, &target->tracks[i].values, target->targetNode, target->tracks[i].channel);
    }
    return 1;
}

struct Clip* anim_make_clip_transition(struct Clip* target, float duration) {
    struct Clip* ret;

    if ((ret = malloc(sizeof(*ret)))) {
        anim_clip_init(ret);
        if ((ret->animations = malloc(target->numAnimations * sizeof(struct Animation)))) {
            unsigned int i;
            ret->numAnimations = target->numAnimations;
            ret->duration = duration;
            ret->mode = CLIP_FORWARD;
            ret->loop = 0;
            ret->oneShot = 1;
            for (i = 0; i < ret->numAnimations; i++) {
                if (!make_anim_transition(&ret->animations[i], &target->animations[i], duration)) break;
            }
            if (i == ret->numAnimations) return ret;
            ret->numAnimations = i;
        }
        anim_clip_free(ret);
        free(ret);
    }
    return NULL;
}
