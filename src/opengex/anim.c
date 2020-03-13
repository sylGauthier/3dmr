#include "liboddl/liboddl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <game/animation/animation.h>

#include "anim.h"
#include "clip.h"
#include "common.h"
#include "context.h"
#include "transform.h"

static float track_duration(const struct Track* track) {
    if (!track->numKeys) return 0.;

    switch (track->times.curveType) {
        case TRACK_LINEAR:
            return track->times.values.linear[track->numKeys - 1];
        case TRACK_BEZIER:
            return track->times.values.bezier[track->numKeys - 1][0];
        default:
            return 0;
    }
}

static float anim_duration(const struct Animation* anim) {
    unsigned int i;
    float duration = 0.;

    for (i = 0; i < TRACK_NB_TYPES; i++) {
        float tmp = track_duration(&anim->tracks[i]);
        if (tmp > duration) duration = tmp;
    }
    return duration;
}

static int infer_target_type(const struct ODDLStructure* target, enum TrackTargetType* type) {
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
static void anim_up_z_to_y(enum TrackTargetType* type, enum TrackKeyType keyType, int* negValues) {
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
        anim->tracks[TRACK_X_POS].values.values.linear[i] = pos[0];
        anim->tracks[TRACK_Y_POS].values.values.linear[i] = pos[1];
        anim->tracks[TRACK_Z_POS].values.values.linear[i] = pos[2];
        anim->tracks[TRACK_X_SCALE].values.values.linear[i] = scale[0];
        anim->tracks[TRACK_Y_SCALE].values.values.linear[i] = scale[1];
        anim->tracks[TRACK_Z_SCALE].values.values.linear[i] = scale[2];
        anim->tracks[TRACK_W_QUAT].values.values.linear[i] = quat[0];
        anim->tracks[TRACK_X_QUAT].values.values.linear[i] = quat[1];
        anim->tracks[TRACK_Y_QUAT].values.values.linear[i] = quat[2];
        anim->tracks[TRACK_Z_QUAT].values.values.linear[i] = quat[3];
    }
    return 1;
}

static int parse_linear_key(struct AnimCurve* curve, unsigned int numKeys, const struct ODDLStructure* cur, int negValues) {
    struct ODDLStructure* sub;
    unsigned int i;

    if (!cur) {
        fprintf(stderr, "Error: Track: missing Key\n");
        return 0;
    }
    if (cur->nbStructures != 1) {
        fprintf(stderr, "Error: Key: must contain exactly one substructure\n");
        return 0;
    }
    if (!ogex_check_struct(sub = cur->structures[0], "Key", TYPE_FLOAT32, numKeys, 1)) return 0;
    memcpy(curve->values.linear, sub->dataList, numKeys * sizeof(float));
    if (negValues) {
        for (i = 0; i < sub->nbVec; i++) curve->values.linear[i] = -curve->values.linear[i];
    }
    return 1;
}

static int parse_bezier_key(struct AnimCurve* curve, unsigned int numKeys, const struct ODDLStructure* vals, const struct ODDLStructure* mControl, const struct ODDLStructure* pControl, int negValues) {
    struct ODDLStructure *subVal, *subPCtrl, *subMCtrl;
    float *valArray, *mCtrlArray, *pCtrlArray;
    Vec3* bezierArray;
    unsigned int i;

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
    bezierArray = curve->values.bezier;
    valArray = subVal->dataList;
    mCtrlArray = subMCtrl->dataList;
    pCtrlArray = subPCtrl->dataList;
    for (i = 0; i < numKeys; i++) {
        bezierArray[i][0] = valArray[i];
        bezierArray[i][1] = mCtrlArray[i];
        bezierArray[i][2] = pCtrlArray[i];
    }
    if (negValues) {
        for (i = 0; i < subVal->nbVec; i++) scale3v(bezierArray[i], -1);
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

static int parse_key(const struct OgexContext* context, struct Animation* anim, enum TrackTargetType type, unsigned int numKeys, enum TrackKeyType keyType, const struct ODDLStructure* cur) {
    struct ODDLStructure *valKey = NULL, *mControlKey = NULL, *pControlKey = NULL;
    struct ODDLProperty* prop;
    struct AnimCurve* curve;
    enum TrackCurve curveType = TRACK_LINEAR;
    unsigned int i;

    if ((prop = oddl_get_property(cur, "curve"))) {
        if (!strcmp(prop->str, "linear")) {
            curveType = TRACK_LINEAR;
        } else if (!strcmp(prop->str, "bezier")) {
            curveType = TRACK_BEZIER;
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
        enum TrackTargetType xfrmtks[] = {TRACK_X_POS, TRACK_Y_POS, TRACK_Z_POS, TRACK_X_SCALE, TRACK_Y_SCALE, TRACK_Z_SCALE, TRACK_W_QUAT, TRACK_X_QUAT, TRACK_Y_QUAT, TRACK_Z_QUAT};
        switch (keyType) {
            case TRACK_KEY_TIME:
                anim_curve_free(curve = &anim->tracks[0].times);
                if (!anim_curve_init(curve, curveType, numKeys)) {
                    anim_curve_init(curve, TRACK_CURVE_NONE, numKeys);
                    fprintf(stderr, "Error: Key: could not allocate memory for new array\n");
                    return 0;
                }
                for (i = 1; i < sizeof(xfrmtks) / sizeof(*xfrmtks); i++) {
                    anim_curve_copy(&anim->tracks[xfrmtks[i]].times, &anim->tracks[0].times);
                    anim->tracks[xfrmtks[i]].numKeys = numKeys;
                }
                switch (curveType) {
                    case TRACK_LINEAR: return parse_linear_key(curve, numKeys, valKey, 0);
                    case TRACK_BEZIER: return parse_bezier_key(curve, numKeys, valKey, mControlKey, pControlKey, 0);
                    default:;
                }
                break;
            case TRACK_KEY_VALUE:
                for (i = 0; i < sizeof(xfrmtks) / sizeof(*xfrmtks); i++) {
                    anim_curve_free(curve = &anim->tracks[xfrmtks[i]].values);
                    if (!anim_curve_init(curve, curveType, numKeys)) {
                        anim_curve_init(curve, TRACK_CURVE_NONE, numKeys);
                        fprintf(stderr, "Error: Key: could not allocate memory for new array\n");
                        return 0;
                    }
                    anim->tracks[xfrmtks[i]].numKeys = numKeys;
                }
                switch (curveType) {
                    case TRACK_LINEAR: return parse_m4_values_linear(context, anim, valKey, numKeys);
                    default:;
                }
                fprintf(stderr, "Error: Key: unsupported curve type for Transform target\n");
                break;
        }
    } else {
        int negValues = 0;
        if (context->up == AXIS_Z) anim_up_z_to_y(&type, keyType, &negValues);
        switch (keyType) {
            case TRACK_KEY_TIME:  curve = &anim->tracks[type].times; break;
            case TRACK_KEY_VALUE: curve = &anim->tracks[type].values; break;
            default: return 0;
        }
        anim_curve_free(curve);
        if (!anim_curve_init(curve, curveType, numKeys)) {
            anim_curve_init(curve, TRACK_CURVE_NONE, numKeys);
            fprintf(stderr, "Error: Key: could not allocate memory for new array\n");
            return 0;
        }
        anim->tracks[type].numKeys = numKeys;
        switch (curveType) {
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
    enum TrackTargetType tgtType;
    unsigned int i, numKeys;

    if (!(prop = oddl_get_property(cur, "target"))) {
        fprintf(stderr, "Error: Track: Track must have a target property\n");
        return 0;
    }
    if (!prop->ref.ref) {
        fprintf(stderr, "Error: Track: invalid ref\n");
        return 0;
    }
    if (!(infer_target_type(prop->ref.ref, &tgtType))) {
        fprintf(stderr, "Error: Track: could not infer Track's target type\n");
        return 0;
    }

    switch (tgtType) {
        case TRACK_X_POS:
        case TRACK_Y_POS:
        case TRACK_Z_POS:
            anim->flags |= TRACKING_POS;
            break;
        case TRACK_X_SCALE:
        case TRACK_Y_SCALE:
        case TRACK_Z_SCALE:
            anim->flags |= TRACKING_SCALE;
            break;
        case TRACK_X_ROT:
        case TRACK_Y_ROT:
        case TRACK_Z_ROT:
            anim->flags |= TRACKING_ROT;
            break;
        case TRACK_W_QUAT:
        case TRACK_X_QUAT:
        case TRACK_Y_QUAT:
        case TRACK_Z_QUAT:
            anim->flags |= TRACKING_QUAT;
            break;
        case TRACK_TRANSFORM:
            anim->flags |= TRACKING_POS | TRACKING_SCALE | TRACKING_QUAT;
            break;
        default:;
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
    return parse_key(context, anim, tgtType, numKeys, TRACK_KEY_TIME, time)
        && parse_key(context, anim, tgtType, numKeys, TRACK_KEY_VALUE, value);
}

int ogex_parse_animation(struct OgexContext* context, struct Node* node, const struct ODDLStructure* cur) {
    unsigned int i, nbTracks = 0, duration = 0, clipIdx = 0;
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
    duration = 1000 * anim_duration(newAnim);
    clip->duration = duration > clip->duration ? duration : clip->duration;
    return 1;
}
