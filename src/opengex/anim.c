#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <game/animation/animation.h>

#include "opengex_common.h"

static float track_duration(struct Track* track) {
    float* flVal = track->times.values;
    Vec3* vec3Val = track->times.values;

    if (!track->nbKeys) return 0.;

    switch (track->times.curveType) {
        case TRACK_LINEAR:
            return flVal[track->nbKeys - 1];
        case TRACK_BEZIER:
            return vec3Val[track->nbKeys - 1][0];
        default:
            return 0;
    }
}

static float anim_duration(struct Animation* anim) {
    unsigned int i;
    float duration = 0.;

    for (i = 0; i < TRACK_NB_TYPES; i++) {
        float tmp;

        tmp = track_duration(&anim->tracks[i]);
        if (tmp > duration) duration = tmp;
    }
    return duration;
}

static int infer_target_type(struct ODDLStructure* target, enum TrackTargetType* type) {
    struct ODDLProperty* prop;

    if (!(prop = oddl_get_property(target, "kind")) || !prop->str) {
        if (ogex_get_identifier(target) != OGEX_TRANSFORM) {
            fprintf(stderr, "Error: could not infer target type, invalid target type or missing 'kind' property\n");
            return 0;
        }
        *type = TRACK_TRANSFORM;
        return 1;
    }
    switch (ogex_get_identifier(target)) {
        case OGEX_TRANSLATION:
            if (!strcmp(prop->str, "x"))        *type = TRACK_X_POS;
            else if (!strcmp(prop->str, "y"))   *type = TRACK_Y_POS;
            else if (!strcmp(prop->str, "z"))   *type = TRACK_Z_POS;
            else return 0;
            break;
        case OGEX_SCALE:
            if (!strcmp(prop->str, "x"))        *type = TRACK_X_SCALE;
            else if (!strcmp(prop->str, "y"))   *type = TRACK_Y_SCALE;
            else if (!strcmp(prop->str, "z"))   *type = TRACK_Z_SCALE;
            else return 0;
            break;
        case OGEX_ROTATION:
            if (!strcmp(prop->str, "x"))        *type = TRACK_X_ROT;
            else if (!strcmp(prop->str, "y"))   *type = TRACK_Y_ROT;
            else if (!strcmp(prop->str, "z"))   *type = TRACK_Z_ROT;
            else return 0;
            break;
        default:
            fprintf(stderr, "Error: can't infer target type, invalid target structure\n");
            return 0;
    }
    return 1;
}

enum TrackKeyType {
    TRACK_KEY_TIME,
    TRACK_KEY_VALUE
};

/* When the animation target is the transform matrix itself, instead of doing a costly matrix interpolation
 * we simply convert every single transform key into the regular (pos,scale,rot) channels and we animate all of
 * them simultaneously.
 */
static int parse_m4_track(struct OgexContext* context, struct Animation* anim, struct ODDLStructure* cur) {
    unsigned int i;
    Mat4* mat = cur->dataList;

    if (cur->vecSize != 16) {
        fprintf(stderr, "Error: Key: Transform data must be float[16]\n");
        return 0;
    }
    for (i = 0; i < TRACK_NB_TYPES; i++) {
        anim->tracks[i].values.curveType = TRACK_LINEAR;
        if (anim->tracks[i].nbKeys && anim->tracks[i].nbKeys != cur->nbVec) {
            fprintf(stderr, "Error: Key: invalid number of key values\n");
            return 0;
        }
        anim->tracks[i].nbKeys = cur->nbVec;
        if (!(anim->tracks[i].values.values = malloc(cur->nbVec * sizeof(float)))) {
            fprintf(stderr, "Error: Key: could not allocate memory for Transform data\n");
            while (i) free(anim->tracks[--i].values.values);
            return 0;
        }
    }
    for (i = 0; i < cur->nbVec; i++) {
        Vec3 pos, scale;
        Quaternion quat;
        if (context->up == AXIS_Z) swap_yz(mat[i]);
        if (!extract_scale(scale, mat[i])) {
            fprintf(stderr, "Error: Key: invalid Transform (null scale)\n");
            for (i = 0; i < TRACK_NB_TYPES; i++) free(anim->tracks[i].values.values);
            return 0;
        }
        memcpy(pos, mat[i][3], sizeof(Vec3));
        quaternion_from_mat4(quat, MAT_CONST_CAST(mat[i]));
        ((float*)(anim->tracks[TRACK_X_POS].values.values))[i] = pos[0];
        ((float*)(anim->tracks[TRACK_Y_POS].values.values))[i] = pos[1];
        ((float*)(anim->tracks[TRACK_Z_POS].values.values))[i] = pos[2];
        ((float*)(anim->tracks[TRACK_X_SCALE].values.values))[i] = scale[0];
        ((float*)(anim->tracks[TRACK_Y_SCALE].values.values))[i] = scale[1];
        ((float*)(anim->tracks[TRACK_Z_SCALE].values.values))[i] = scale[2];
        ((float*)(anim->tracks[TRACK_W_QUAT].values.values))[i] = quat[0];
        ((float*)(anim->tracks[TRACK_X_QUAT].values.values))[i] = quat[1];
        ((float*)(anim->tracks[TRACK_Y_QUAT].values.values))[i] = quat[2];
        ((float*)(anim->tracks[TRACK_Z_QUAT].values.values))[i] = quat[3];
    }
    return 1;
}

static int parse_linear_key(struct OgexContext* context, struct Animation* anim, enum TrackTargetType type, enum TrackKeyType keyType, struct ODDLStructure* cur) {
    struct ODDLStructure* sub;
    float* array = NULL;
    unsigned int i;

    if (cur->nbStructures != 1) {
        fprintf(stderr, "Error: Key: must contain exactly one substructure\n");
        return 0;
    }
    sub = cur->structures[0];
    if (sub->type != TYPE_FLOAT32) {
        fprintf(stderr, "Error: Key: substructure must be of type float\n");
        return 0;
    }
    if (type != TRACK_TRANSFORM && sub->vecSize != 1) {
        fprintf(stderr, "Error: Key: multidimensional values not supported\n");
        return 0;
    }
    if (sub->nbVec < 1) {
        fprintf(stderr, "Error: Key: must have at least one float value\n");
        return 0;
    }
    if (type != TRACK_TRANSFORM && anim->tracks[type].nbKeys && anim->tracks[type].nbKeys != sub->nbVec) {
        fprintf(stderr, "Error: Key: inconsistent nb of key values, expected %d but got %d\n",
                anim->tracks[type].nbKeys, sub->nbVec);
        return 0;
    }
    if (!(array = malloc(sub->nbVec * sizeof(float)))) {
        fprintf(stderr, "Error: Key: could not allocate memory for new array\n");
        return 0;
    }
    memcpy(array, sub->dataList, sub->nbVec * sizeof(float));
    if (context->up == AXIS_Z) { /* What a great f***ing idea to have arbitrary up axis, thanks Obama! */
        if (type == TRACK_Y_POS) {
            if (keyType == TRACK_KEY_VALUE) {
                unsigned int i;
                for (i = 0; i < sub->nbVec; i++) array[i] = -array[i];
            }
            type = TRACK_Z_POS;
        } else if (type == TRACK_Y_ROT) {
            if (keyType == TRACK_KEY_VALUE) {
                unsigned int i;
                for (i = 0; i < sub->nbVec; i++) array[i] = -array[i];
            }
            type = TRACK_Z_ROT;
        } else if (type == TRACK_Y_SCALE) type = TRACK_Z_SCALE;
        else if (type == TRACK_Z_POS) type = TRACK_Y_POS;
        else if (type == TRACK_Z_ROT) type = TRACK_Y_ROT;
        else if (type == TRACK_Z_SCALE) type = TRACK_Y_SCALE;
    }
    switch (keyType) {
        case TRACK_KEY_TIME:
            if (type == TRACK_TRANSFORM) {
                for (i = 0; i < TRACK_NB_TYPES; i++) {
                    anim->tracks[i].times.values = array;
                    anim->tracks[i].times.curveType = TRACK_LINEAR;
                    anim->tracks[i].times.sharedValues = i > 0;
                    anim->tracks[i].nbKeys = sub->nbVec;
                }
            } else {
                anim->tracks[type].times.values = array;
                anim->tracks[type].times.curveType = TRACK_LINEAR;
                anim->tracks[type].nbKeys = sub->nbVec;
            }
            return 1;
        case TRACK_KEY_VALUE:
            if (type == TRACK_TRANSFORM) {
                if (!parse_m4_track(context, anim, sub)) {
                    free(array);
                    return 0;
                }
                free(array);
                return 1;
            } else {
                anim->tracks[type].values.values = array;
                anim->tracks[type].values.curveType = TRACK_LINEAR;
                anim->tracks[type].nbKeys = sub->nbVec;
                return 1;
            }
        default:
            free(array);
            return 0;
    }
    return 0;
}

static int parse_bezier_key(struct OgexContext* context, struct Animation* anim, enum TrackTargetType type, enum TrackKeyType keyType, struct ODDLStructure* vals, struct ODDLStructure* mControl, struct ODDLStructure* pControl) {
    struct ODDLStructure *subVal, *subPCtrl, *subMCtrl;
    unsigned int i;
    float *valArray, *mCtrlArray, *pCtrlArray;
    Vec3* bezierArray;

    if (vals->nbStructures != 1 || mControl->nbStructures != 1 || pControl->nbStructures != 1) {
        fprintf(stderr, "Error: Bezier Key: must contain exactly one substructure\n");
        return 0;
    }
    subVal = vals->structures[0];
    subMCtrl = mControl->structures[0];
    subPCtrl = pControl->structures[0];
    if (subVal->type != TYPE_FLOAT32 || subMCtrl->type != TYPE_FLOAT32 || subPCtrl->type != TYPE_FLOAT32) {
        fprintf(stderr, "Error: Bezier Key: substructure must be of type float\n");
        return 0;
    }
    if (subVal->vecSize != 1 || subMCtrl->vecSize != 1 || subPCtrl->vecSize != 1) {
        fprintf(stderr, "Error: Bezier Key: multidimensional values not supported\n");
        return 0;
    }
    if (subVal->nbVec < 1 || subMCtrl->nbVec < 1 || subPCtrl->nbVec < 1) {
        fprintf(stderr, "Error: Bezier Key: must have at least one float value\n");
        return 0;
    }
    if (anim->tracks[type].nbKeys && (subVal->nbVec != anim->tracks[type].nbKeys
                                   || subMCtrl->nbVec != anim->tracks[type].nbKeys
                                   || subPCtrl->nbVec != anim->tracks[type].nbKeys)) {
        fprintf(stderr, "Error: Bezier Key: inconsistent nb of key values, expected %d\n", anim->tracks[type].nbKeys);
        return 0;
    }
    if (subVal->nbVec != subMCtrl->nbVec || subVal->nbVec != subPCtrl->nbVec) {
        fprintf(stderr, "Error: Bezier Key: inconsistent nb of key values\n");
        return 0;
    }
    if (!(bezierArray = malloc(3 * subVal->nbVec * sizeof(float)))) {
        fprintf(stderr, "Error: Bezier Key: could not allocate memory for new array\n");
        return 0;
    }
    valArray = subVal->dataList;
    mCtrlArray = subMCtrl->dataList;
    pCtrlArray = subPCtrl->dataList;
    for (i = 0; i < subVal->nbVec; i++) {
        bezierArray[i][0] = valArray[i];
        bezierArray[i][1] = mCtrlArray[i];
        bezierArray[i][2] = pCtrlArray[i];
    }
    if (context->up == AXIS_Z) {
        if (type == TRACK_Y_POS) {
            if (keyType == TRACK_KEY_VALUE) {
                unsigned int i;
                for (i = 0; i < subVal->nbVec; i++) scale3v(bezierArray[i], -1);
            }
            type = TRACK_Z_POS;
        } else if (type == TRACK_Y_ROT) {
            if (keyType == TRACK_KEY_VALUE) {
                unsigned int i;
                for (i = 0; i < subVal->nbVec; i++) scale3v(bezierArray[i], -1);
            }
            type = TRACK_Z_ROT;
        } else if (type == TRACK_Y_SCALE) type = TRACK_Z_SCALE;
        else if (type == TRACK_Z_POS) type = TRACK_Y_POS;
        else if (type == TRACK_Z_ROT) type = TRACK_Y_ROT;
        else if (type == TRACK_Z_SCALE) type = TRACK_Y_SCALE;
    }
    switch (keyType) {
        case TRACK_KEY_TIME:
            anim->tracks[type].times.values = bezierArray;
            anim->tracks[type].times.curveType = TRACK_BEZIER;
            anim->tracks[type].nbKeys = subVal->nbVec;
            return 1;
        case TRACK_KEY_VALUE:
            anim->tracks[type].values.values = bezierArray;
            anim->tracks[type].values.curveType = TRACK_BEZIER;
            anim->tracks[type].nbKeys = subVal->nbVec;
            return 1;
        default:
            free(bezierArray);
            return 0;
    }
    return 0;
}

static int parse_key(struct OgexContext* context, struct Animation* anim, enum TrackTargetType type, enum TrackKeyType keyType, struct ODDLStructure* cur) {
    struct ODDLStructure *valKey = NULL, *mControlKey = NULL, *pControlKey = NULL;
    struct ODDLProperty* prop;
    enum TrackCurve curve = TRACK_LINEAR;
    unsigned int i;

    if ((prop = oddl_get_property(cur, "curve"))) {
        if (!strcmp(prop->str, "linear")) curve = TRACK_LINEAR;
        else if (!strcmp(prop->str, "bezier")) curve = TRACK_BEZIER;
        else fprintf(stderr, "Warning: Value: unsupported curve type: %s\n", prop->str);
    }

    for (i = 0; i < cur->nbStructures; i++) {
        struct ODDLStructure* tmp = cur->structures[i];

        switch (ogex_get_identifier(tmp)) {
            case OGEX_KEY:
                if ((prop = oddl_get_property(tmp, "kind"))) {
                    if (!strcmp(prop->str, "-control")) mControlKey = tmp;
                    if (!strcmp(prop->str, "+control")) pControlKey = tmp;
                } else {
                    valKey = tmp;
                }
                break;
            default:
                break;
        }
    }
    switch (curve) {
        case TRACK_LINEAR:
            if (valKey) {
                if (!parse_linear_key(context, anim, type, keyType, valKey)) {
                    return 0;
                }
            } else {
                fprintf(stderr, "Error: Track: missing Key\n");
                return 0;
            }
            break;
        case TRACK_BEZIER:
            if (valKey && mControlKey && pControlKey) {
                if (!parse_bezier_key(context, anim, type, keyType, valKey, mControlKey, pControlKey)) {
                    return 0;
                }
            } else {
                fprintf(stderr, "Error: Track: missing Key, Bezier requires 3 Keys\n");
                return 0;
            }
            break;
        default:
            fprintf(stderr, "Error: Track: unknown curve type\n");
            return 0;

    }
    return 1;
}

static int parse_track(struct OgexContext* context, struct Animation* anim, struct ODDLStructure* cur) {
    unsigned int hasTime = 0, hasValue = 0, i;
    enum TrackTargetType tgtType;
    struct ODDLProperty* prop;

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
            anim_track_pos(anim);
            break;
        case TRACK_X_SCALE:
        case TRACK_Y_SCALE:
        case TRACK_Z_SCALE:
            anim_track_scale(anim);
            break;
        case TRACK_X_ROT:
        case TRACK_Y_ROT:
        case TRACK_Z_ROT:
            anim_track_rot(anim);
            break;
        case TRACK_W_QUAT:
        case TRACK_X_QUAT:
        case TRACK_Y_QUAT:
        case TRACK_Z_QUAT:
            anim_track_quat(anim);
            break;
        case TRACK_TRANSFORM:
            anim_track_transform(anim);
            break;
        default:
            break;
    }

    for (i = 0; i < cur->nbStructures; i++) {
        struct ODDLStructure* tmp = cur->structures[i];

        switch (ogex_get_identifier(tmp)) {
            case OGEX_TIME:
                if (hasTime) {
                    fprintf(stderr, "Error: Track: multiple time key arrays\n");
                    return 0;
                }
                if (!(parse_key(context, anim, tgtType, TRACK_KEY_TIME, tmp))) {
                    return 0;
                }
                hasTime = 1;
                break;
            case OGEX_VALUE:
                if (hasValue) {
                    fprintf(stderr, "Error: Track: multiple value key arrays\n");
                    return 0;
                }
                if (!(parse_key(context, anim, tgtType, TRACK_KEY_VALUE, tmp))) {
                    return 0;
                }
                hasValue = 1;
            default:
                break;
        }
    }
    return 1;
}

int ogex_parse_animation(struct OgexContext* context, struct Node* node, struct ODDLStructure* cur) {
    unsigned int i, nbTracks = 0, duration = 0;
    struct Clip* clip = NULL;
    struct Animation* newAnim = NULL;

    if (!context->metadata->nbClips) {
        if (!(context->metadata->clips = malloc(sizeof(struct Clip)))) {
            fprintf(stderr, "Error: Animation: could not allocate memory for new Clip\n");
            return 0;
        }
        context->metadata->nbClips++;
        memset(context->metadata->clips, 0, sizeof(struct Clip));
    }
    clip = context->metadata->clips;
    if (anim_clip_new_anim(clip, node) < 0) {
        fprintf(stderr, "Error: Animation: could not allocate memory for new Animation\n");
        return 0;
    }
    newAnim = clip->animations + (clip->nbAnimations - 1);

    for (i = 0; i < cur->nbStructures; i++) {
        struct ODDLStructure* tmp = cur->structures[i];

        switch (ogex_get_identifier(tmp)) {
            case OGEX_TRACK:
                if (!parse_track(context, newAnim, tmp)) {
                    return 0;
                }
                nbTracks++;
                break;
            default:
                break;
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
