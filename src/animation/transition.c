#include <stdlib.h>

#include <3dmr/animation/animation.h>

static void make_curve_transition(struct AnimCurve* dest, const struct AnimCurve* targetCurve, struct Node* targetNode, enum TrackTargetType t) {
    Vec3 eulerRot;
    if (t == TRACK_X_ROT || t == TRACK_Y_ROT || t == TRACK_Z_ROT) {
        quaternion_to_xyz(eulerRot, targetNode->orientation);
    }
    switch (t) {
        case TRACK_X_POS:
            dest->values.linear[0] = targetNode->position[0];
            break;
        case TRACK_Y_POS:
            dest->values.linear[0] = targetNode->position[1];
            break;
        case TRACK_Z_POS:
            dest->values.linear[0] = targetNode->position[2];
            break;
        case TRACK_X_SCALE:
            dest->values.linear[0] = targetNode->scale[0];
            break;
        case TRACK_Y_SCALE:
            dest->values.linear[0] = targetNode->scale[1];
            break;
        case TRACK_Z_SCALE:
            dest->values.linear[0] = targetNode->scale[2];
            break;
        case TRACK_X_ROT:
            dest->values.linear[0] = eulerRot[0];
            break;
        case TRACK_Y_ROT:
            dest->values.linear[0] = eulerRot[1];
            break;
        case TRACK_Z_ROT:
            dest->values.linear[0] = eulerRot[2];
            break;
        case TRACK_W_QUAT:
            dest->values.linear[0] = targetNode->orientation[0];
            break;
        case TRACK_X_QUAT:
            dest->values.linear[0] = targetNode->orientation[1];
            break;
        case TRACK_Y_QUAT:
            dest->values.linear[0] = targetNode->orientation[2];
            break;
        case TRACK_Z_QUAT:
            dest->values.linear[0] = targetNode->orientation[3];
            break;
        default:
            break;
    }
    switch (targetCurve->curveType) {
        case TRACK_LINEAR:
            dest->values.linear[1] = targetCurve->values.linear[0];
            break;
        case TRACK_BEZIER:
            dest->values.linear[1] = targetCurve->values.bezier[0][0];
            break;
        default:
            break;
    }
}

static int make_anim_transition(struct Animation* transition, const struct Animation* target, float duration) {
    unsigned int i, f;

    if (!anim_animation_init(transition, target->targetNode)) return 0;
    transition->flags = target->flags;
    for (f = 0; f < TRACK_NB_TYPES && !target->tracks[f].numKeys; f++);
    if (f == TRACK_NB_TYPES) return 1;
    if (!anim_track_init(&transition->tracks[f], TRACK_LINEAR, TRACK_LINEAR, 2)) {
        anim_animation_free(transition);
        return 0;
    }
    transition->tracks[f].times.values.linear[0] = 0;
    transition->tracks[f].times.values.linear[1] = duration;
    make_curve_transition(&transition->tracks[f].values, &target->tracks[f].values, target->targetNode, f);
    for (i = f + 1; i < TRACK_NB_TYPES; i++) {
        if (target->tracks[i].numKeys) {
            transition->tracks[i].numKeys = 2;
            anim_curve_copy(&transition->tracks[i].times, &transition->tracks[f].times);
            if (!anim_curve_init(&transition->tracks[i].values, TRACK_LINEAR, 2)) {
                anim_animation_free(transition);
                return 0;
            }
            make_curve_transition(&transition->tracks[i].values, &target->tracks[i].values, target->targetNode, i);
        }
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
