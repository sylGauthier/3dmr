#include "liboddl/liboddl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <3dmr/animation/animation.h>

#include "anim.h"
#include "clip.h"
#include "common.h"
#include "context.h"
#include "transform.h"

static float track_duration(const struct Track* track) {
    if (!track->numKeys) return 0.;

    switch (track->times.interp) {
        case TRACK_LINEAR:
            return track->times.values[track->numKeys - 1];
        case TRACK_BEZIER:
            return track->times.values[3 * (track->numKeys - 1)];
        default:
            return 0;
    }
}

static float anim_duration(const struct Animation* anim) {
    unsigned int i;
    float duration = 0.;

    for (i = 0; i < anim->numTracks; i++) {
        float tmp = track_duration(&anim->tracks[i]);
        if (tmp > duration) duration = tmp;
    }
    return duration;
}

static int infer_target_type(const struct ODDLStructure* target, enum TrackChannel* type) {
    struct ODDLProperty* prop;

    if (!(prop = oddl_get_property(target, "kind")) || !prop->str) {
        if (!target->identifier || strcmp(target->identifier, "Transform")) {
            fprintf(stderr, "Error: could not infer target type, invalid target type or missing 'kind' property\n");
            return 0;
        }
        *type = TRACK_TRANSFORM;
        return 1;
    }
    if (prop->str[0] < 'x' || prop->str[0] > 'z' || prop->str[1]) {
        fprintf(stderr, "Error: can't infer target type, invalid 'kind' property\n");
        return 0;
    } else if (!target->identifier) {
        fprintf(stderr, "Error: can't infer target type, invalid target structure\n");
        return 0;
    } else if (!strcmp(target->identifier, "Translation")) {
        *type = TRACK_X_POS + (prop->str[0] - 'x');
    } else if (!strcmp(target->identifier, "Scale")) {
        *type = TRACK_X_SCALE + (prop->str[0] - 'x');
    } else if (!strcmp(target->identifier, "Rotation")) {
        *type = TRACK_X_ROT + (prop->str[0] - 'x');
    } else {
        fprintf(stderr, "Error: can't infer target type, invalid target structure\n");
        return 0;
    }
    return 1;
}

enum TrackKeyType {
    TRACK_KEY_TIME,
    TRACK_KEY_VALUE
};

/* What a great f***ing idea to have arbitrary up axis, thanks Obama! */
static void anim_up_z_to_y(enum TrackChannel* type, enum TrackKeyType keyType, int* negValues) {
    *negValues = 0;
    switch (*type) {
        case TRACK_Y_POS:   *type = TRACK_Z_POS; *negValues = (keyType == TRACK_KEY_VALUE); break;
        case TRACK_Y_ROT:   *type = TRACK_Z_ROT; *negValues = (keyType == TRACK_KEY_VALUE); break;
        case TRACK_Y_SCALE: *type = TRACK_Z_SCALE; break;
        case TRACK_Z_POS:   *type = TRACK_Y_POS; break;
        case TRACK_Z_ROT:   *type = TRACK_Y_ROT; break;
        case TRACK_Z_SCALE: *type = TRACK_Y_SCALE; break;
        default:;
    }
}

/* When the animation target is the transform matrix itself, instead of doing a costly matrix interpolation
 * we simply convert every single transform key into the regular (pos,scale,rot) channels and we animate all of
 * them simultaneously.
 */
static int parse_m4_values_linear(const struct OgexContext* context, struct Animation* anim, const struct ODDLStructure* cur, unsigned int numKeys) {
    struct ODDLStructure* sub;
    Mat4* mat;
    unsigned int i;

    if (!cur) {
        fprintf(stderr, "Error: Track: missing Key\n");
        return 0;
    }
    if (cur->nbStructures != 1) {
        fprintf(stderr, "Error: Key: must contain exactly one substructure\n");
        return 0;
    }
    if (!ogex_check_struct(sub = cur->structures[0], "Key", TYPE_FLOAT32, numKeys, 16)) return 0;
    mat = sub->dataList;
    for (i = 0; i < numKeys; i++) {
        Vec3 pos, scale;
        Quaternion quat;
        if (context->up == AXIS_Z) ogex_swap_yz_mat(mat[i]);
        if (!extract_scale(scale, mat[i])) {
            fprintf(stderr, "Error: Key: invalid Transform (null scale)\n");
            return 0;
        }
        memcpy(pos, mat[i][3], sizeof(Vec3));
        quaternion_from_mat4(quat, MAT_CONST_CAST(mat[i]));
        memcpy(&anim->tracks[0].values.values[3 * i], pos, sizeof(pos));
        memcpy(&anim->tracks[1].values.values[3 * i], scale, sizeof(scale));
        memcpy(&anim->tracks[2].values.values[4 * i], quat, sizeof(quat));
    }
    return 1;
}

static int parse_linear_key(struct AnimCurve* curve, unsigned int numKeys, const struct ODDLStructure* cur, int negValues) {
    struct ODDLStructure* sub;
    unsigned int i;

    if (curve->numComponent != 1) {
        fprintf(stderr, "Error: Track: invalid component type\n");
        return 0;
    }
    if (!cur) {
        fprintf(stderr, "Error: Track: missing Key\n");
        return 0;
    }
    if (cur->nbStructures != 1) {
        fprintf(stderr, "Error: Key: must contain exactly one substructure\n");
        return 0;
    }
    if (!ogex_check_struct(sub = cur->structures[0], "Key", TYPE_FLOAT32, numKeys, 1)) return 0;
    memcpy(curve->values, sub->dataList, numKeys * sizeof(float));
    if (negValues) {
        for (i = 0; i < sub->nbVec; i++) curve->values[i] = -curve->values[i];
    }
    return 1;
}

static int parse_bezier_key(struct AnimCurve* curve, unsigned int numKeys, const struct ODDLStructure* vals, const struct ODDLStructure* mControl, const struct ODDLStructure* pControl, int negValues) {
    struct ODDLStructure *subVal, *subPCtrl, *subMCtrl;
    float *valArray, *mCtrlArray, *pCtrlArray, *bezierArray;
    unsigned int i;

    if (curve->numComponent != 1) {
        fprintf(stderr, "Error: Track: invalid component type\n");
        return 0;
    }
    if (!(vals && mControl && pControl)) {
        fprintf(stderr, "Error: Track: missing Key, Bezier requires 3 Keys\n");
        return 0;
    }
    if (vals->nbStructures != 1 || mControl->nbStructures != 1 || pControl->nbStructures != 1) {
        fprintf(stderr, "Error: Bezier Key: must contain exactly one substructure\n");
        return 0;
    }
    if (!ogex_check_struct(subVal = vals->structures[0], "Bezier Key", TYPE_FLOAT32, numKeys, 1)) return 0;
    if (!ogex_check_struct(subMCtrl = mControl->structures[0], "Bezier Key", TYPE_FLOAT32, numKeys, 1)) return 0;
    if (!ogex_check_struct(subPCtrl = pControl->structures[0], "Bezier Key", TYPE_FLOAT32, numKeys, 1)) return 0;
    bezierArray = curve->values;
    valArray = subVal->dataList;
    mCtrlArray = subMCtrl->dataList;
    pCtrlArray = subPCtrl->dataList;
    for (i = 0; i < numKeys; i++) {
        bezierArray[3 * i] = valArray[i];
        bezierArray[3 * i + 1] = mCtrlArray[i];
        bezierArray[3 * i + 2] = pCtrlArray[i];
    }
    if (negValues) {
        for (i = 0; i < subVal->nbVec; i++) scale3v(&bezierArray[3 * i], -1);
    }
    return 1;
}

static int get_num_keys(const struct ODDLStructure* cur) {
    struct ODDLStructure* valKey = NULL;
    struct ODDLProperty* prop;
    unsigned int i;

    for (i = 0; i < cur->nbStructures; i++) {
        struct ODDLStructure* tmp = cur->structures[i];

        if (tmp->identifier && !strcmp(tmp->identifier, "Key")) {
            if (!(prop = oddl_get_property(tmp, "kind")) || !strcmp(prop->str, "value")) {
                valKey = tmp;
            }
        }
    }
    if (!valKey) {
        fprintf(stderr, "Error: Time/Value: missing value Key\n");
        return 0;
    }
    if (valKey->nbStructures != 1 || !valKey->structures[0]->nbVec) {
        fprintf(stderr, "Error: Time/Value: invalid value Key\n");
        return 0;
    }
    return valKey->structures[0]->nbVec;
}

static struct Track* get_track_channel(struct Animation* anim, enum TrackChannel channel) {
    unsigned int i;
    for (i = 0; i < anim->numTracks; i++) {
        if (anim->tracks[i].channel == channel) return anim->tracks + i;
    }
    return NULL;
}

static int parse_key(const struct OgexContext* context, struct Animation* anim, enum TrackChannel type, unsigned int numKeys, enum TrackKeyType keyType, const struct ODDLStructure* cur) {
    struct ODDLStructure *valKey = NULL, *mControlKey = NULL, *pControlKey = NULL;
    struct ODDLProperty* prop;
    enum TrackInterp interp = TRACK_LINEAR;
    unsigned int i;

    if ((prop = oddl_get_property(cur, "curve"))) {
        if (!strcmp(prop->str, "linear")) {
            interp = TRACK_LINEAR;
        } else if (!strcmp(prop->str, "bezier")) {
            interp = TRACK_BEZIER;
        } else {
            fprintf(stderr, "Error: Track: unsupported curve type: %s\n", prop->str);
            return 0;
        }
    }

    for (i = 0; i < cur->nbStructures; i++) {
        struct ODDLStructure* tmp = cur->structures[i];

        if (tmp->identifier && !strcmp(tmp->identifier, "Key")) {
            if ((prop = oddl_get_property(tmp, "kind"))) {
                if (!strcmp(prop->str, "-control")) mControlKey = tmp;
                if (!strcmp(prop->str, "+control")) pControlKey = tmp;
                if (!strcmp(prop->str, "value")) valKey = tmp;
            } else {
                valKey = tmp;
            }
        }
    }
    if (type == TRACK_TRANSFORM) {
        unsigned int id;
        switch (keyType) {
            case TRACK_KEY_TIME:
                if (!anim->numTracks) {
                    if (       !anim_animation_new_track(anim, TRACK_POS, interp, TRACK_LINEAR, numKeys, &id)
                            || !anim_animation_new_track(anim, TRACK_SCALE, TRACK_INTERP_NONE, TRACK_LINEAR, numKeys, &id)
                            || !anim_animation_new_track(anim, TRACK_QUAT, TRACK_INTERP_NONE, TRACK_LINEAR, numKeys, &id)) {
                        return 0;
                    }
                }
                anim_curve_copy(&anim->tracks[1].times, &anim->tracks[0].times);
                anim_curve_copy(&anim->tracks[2].times, &anim->tracks[0].times);
                switch (interp) {
                    case TRACK_LINEAR: return parse_linear_key(&anim->tracks[0].times, numKeys, valKey, 0);
                    case TRACK_BEZIER: return parse_bezier_key(&anim->tracks[0].times, numKeys, valKey, mControlKey, pControlKey, 0);
                    default:;
                }
                break;
            case TRACK_KEY_VALUE:
                if (!anim->numTracks) {
                    if (       !anim_animation_new_track(anim, TRACK_POS, interp, TRACK_LINEAR, numKeys, &id)
                            || !anim_animation_new_track(anim, TRACK_SCALE, TRACK_INTERP_NONE, TRACK_LINEAR, numKeys, &id)
                            || !anim_animation_new_track(anim, TRACK_QUAT, TRACK_INTERP_NONE, TRACK_LINEAR, numKeys, &id)) {
                        return 0;
                    }
                }
                switch (interp) {
                    case TRACK_LINEAR: return parse_m4_values_linear(context, anim, valKey, numKeys);
                    default:;
                }
                fprintf(stderr, "Error: Key: unsupported curve type for Transform target\n");
                break;
        }
    } else {
        int negValues = 0;
        unsigned int id;
        struct Track* track;
        struct AnimCurve* curve;
        if (context->up == AXIS_Z) anim_up_z_to_y(&type, keyType, &negValues);
        if (!(track = get_track_channel(anim, type))) {
            if (!anim_animation_new_track(anim, type, TRACK_INTERP_NONE, TRACK_INTERP_NONE, numKeys, &id))
                return 0;
            track = &anim->tracks[id];
        }
        switch (keyType) {
            case TRACK_KEY_TIME:  curve = &track->times; break;
            case TRACK_KEY_VALUE: curve = &track->values; break;
            default: return 0;
        }
        if (track->numKeys != numKeys) {
            anim_curve_free(curve);
            if (!anim_curve_init(curve, interp, 1, numKeys)) {
                anim_curve_init(curve, TRACK_INTERP_NONE, 1, numKeys);
                fprintf(stderr, "Error: Key: could not allocate memory for new array\n");
                return 0;
            }
        }
        switch (interp) {
            case TRACK_LINEAR: return parse_linear_key(curve, numKeys, valKey, negValues);
            case TRACK_BEZIER: return parse_bezier_key(curve, numKeys, valKey, mControlKey, pControlKey, negValues);
            default:;
        }
    }
    return 0;
}

static int track_check_unique(const struct ODDLStructure** p, const struct ODDLStructure* v, const char* t) {
    if (*p) {
        fprintf(stderr, "Error: Track: multiple %s key arrays\n", t);
        return 0;
    }
    *p = v;
    return 1;
}

static int parse_track(const struct OgexContext* context, struct Animation* anim, const struct ODDLStructure* cur) {
    const struct ODDLStructure *time = NULL, *value = NULL;
    struct ODDLProperty* prop;
    enum TrackChannel channel;
    unsigned int i, numKeys;

    if (!(prop = oddl_get_property(cur, "target"))) {
        fprintf(stderr, "Error: Track: Track must have a target property\n");
        return 0;
    }
    if (!prop->ref.ref) {
        fprintf(stderr, "Error: Track: invalid ref\n");
        return 0;
    }
    if (!(infer_target_type(prop->ref.ref, &channel))) {
        fprintf(stderr, "Error: Track: could not infer Track's target type\n");
        return 0;
    }

    for (i = 0; i < cur->nbStructures; i++) {
        struct ODDLStructure* tmp = cur->structures[i];

        if (!tmp->identifier) {
            continue;
        } else if (!strcmp(tmp->identifier, "Time")) {
            if (!track_check_unique(&time, tmp, "time")) return 0;
        } else if (!strcmp(tmp->identifier, "Value")) {
            if (!track_check_unique(&value, tmp, "value")) return 0;
        }
    }
    if (!time || !value) {
        fprintf(stderr, "Error: Track: missing Time or Value\n");
        return 0;
    }
    if (!(numKeys = get_num_keys(time))) return 0;
    if (numKeys != get_num_keys(value)) {
        fprintf(stderr, "Error: Track: number of keys mismatch in Time and Value\n");
        return 0;
    }
    return parse_key(context, anim, channel, numKeys, TRACK_KEY_TIME, time)
        && parse_key(context, anim, channel, numKeys, TRACK_KEY_VALUE, value);
}

int ogex_parse_animation(struct OgexContext* context, struct Node* node, const struct ODDLStructure* cur) {
    unsigned int i, nbTracks = 0, clipIdx = 0;
    float duration = 0;
    struct Clip* clip = NULL;
    struct Animation* newAnim = NULL;
    struct ODDLProperty* prop;

    if ((prop = oddl_get_property(cur, "clip"))) {
        clipIdx = prop->llint;
    }
    if (context->numClips <= clipIdx || !(clip = context->clips[clipIdx])) {
        if (!(clip = ogex_create_new_clip(context, clipIdx))) return 0;
    }
    if (!anim_clip_new_anim(clip, node)) {
        fprintf(stderr, "Error: Animation: could not allocate memory for new Animation\n");
        return 0;
    }
    newAnim = clip->animations + (clip->numAnimations - 1);

    for (i = 0; i < cur->nbStructures; i++) {
        struct ODDLStructure* tmp = cur->structures[i];

        if (tmp->identifier && !strcmp(tmp->identifier, "Track")) {
            if (!parse_track(context, newAnim, tmp)) return 0;
            nbTracks++;
        }
    }

    if (!nbTracks) {
        fprintf(stderr, "Error: Animation: Animation should contain at least one Track\n");
        return 0;
    }
    duration = anim_duration(newAnim);
    clip->duration = duration > clip->duration ? duration : clip->duration;
    return 1;
}
