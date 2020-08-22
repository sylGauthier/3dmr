#include <string.h>

#include "gltf.h"

static void infer_clip_duration(struct Clip* clip) {
    unsigned int i, j;

    clip->duration = 0.;
    for (i = 0; i < clip->numAnimations; i++) {
        for (j = 0; j < clip->animations[i].numTracks; j++) {
            if (clip->animations[i].tracks[j].numKeys) {
                /* Time should always be linear with gltf */
                float t = clip->animations[i].tracks[j].times.values[clip->animations[i].tracks[j].numKeys - 1];
                clip->duration = t > clip->duration ? t : clip->duration;
            }
        }
    }
}

static int track_load_vecn(struct GltfContext* context, struct Animation* anim, enum TrackChannel channel,
                           struct GltfAccessor* in, struct GltfAccessor* out, const char* interp) {
    unsigned int i, numKeys, n, newTrack;
    float *inf, *outf;
    enum GltfType type;
    struct Track* track;

    switch (channel) {
        case TRACK_POS:
        case TRACK_SCALE:
            type = GLTF_VEC3;
            n = 3;
            break;
        case TRACK_QUAT:
            type = GLTF_VEC4;
            n = 4;
            break;
        default:
            fprintf(stderr, "Error: gltf: anim: unsupported channel type\n");
            return 0;
    }
    if (strcmp(interp, "LINEAR")) {
        fprintf(stderr, "Error: gltf: anim: only LINEAR supported yet\n");
        return 0;
    }
    if (in->componentType != GLTF_FLOAT || in->type != GLTF_SCALAR) {
        fprintf(stderr, "Error: gltf: anim: in channel has invalid type or component type\n");
        return 0;
    }
    if (out->componentType != GLTF_FLOAT || out->type != type) {
        fprintf(stderr, "Error: gltf: anim: out channel has invalid type or component type\n");
        return 0;
    }
    if (in->count != out->count) {
        fprintf(stderr, "Error: gltf: anim: inconsistent number of input keys and output keys\n");
        return 0;
    }
    numKeys = in->count;
    if (!numKeys) return 1;
    inf = gltf_acc_get_buf(context, in);
    outf = gltf_acc_get_buf(context, out);

    /* TODO: add STEP and CUBIC_SPLINE interpolation methods here */
    if (!anim_animation_new_track(anim, channel, TRACK_LINEAR, TRACK_LINEAR, numKeys, &newTrack)) return 0;
    track = anim->tracks + newTrack;
    memcpy(track->times.values, inf, numKeys * sizeof(float));

    switch (channel) {
        case TRACK_POS:
        case TRACK_SCALE:
            memcpy(track->values.values, outf, numKeys * n * sizeof(float));
            break;
        case TRACK_QUAT:
            for (i = 0; i < numKeys; i++) {
                unsigned int j;
                /* Quaternion, gltf is XYZW, we are WXYZ so we need to swap */
                for (j = 0; j < 4; j++) track->values.values[4 * i + (j + 1) % 4] = outf[4 * i + j];
            }
            break;
        default:
            return 0;
    }
    return 1;
}

static int parse_track(struct GltfContext* context, struct Animation* anim, json_t* channel, json_t* samplers) {
    json_t *tmp, *sampler, *path;
    const char *pathStr, *interp;
    struct GltfAccessor *in, *out;

    /* Get sampler */
    if (!(tmp = json_object_get(channel, "sampler")) || !json_is_integer(tmp)) {
        fprintf(stderr, "Error: gltf: anim: channel: missing sampler\n");
        return 0;
    }
    if (!(sampler = json_array_get(samplers, json_integer_value(tmp)))) {
        fprintf(stderr, "Error: gltf: anim: channel: invalid sampler index\n");
        return 0;
    }

    /* Get input/output accessors and interpolation */
    if (!(tmp = json_object_get(sampler, "input")) || !json_is_integer(tmp)) {
        fprintf(stderr, "Error: gltf: anim: sampler: missing input\n");
        return 0;
    }
    if (json_integer_value(tmp) >= context->numAccessors) {
        fprintf(stderr, "Error: gltf: anim: sampler: invalid input index\n");
        return 0;
    }
    in = context->accessors + json_integer_value(tmp);

    if (!(tmp = json_object_get(sampler, "output")) || !json_is_integer(tmp)) {
        fprintf(stderr, "Error: gltf: anim: sampler: missing output\n");
        return 0;
    }
    if (json_integer_value(tmp) >= context->numAccessors) {
        fprintf(stderr, "Error: gltf: anim: sampler: invalid output index\n");
        return 0;
    }
    out = context->accessors + json_integer_value(tmp);

    if (!(tmp = json_object_get(sampler, "interpolation")) || !(interp = json_string_value(tmp))) {
        fprintf(stderr, "Error: gltf: anim: sampler: missing interpolation\n");
        return 0;
    }

    /* Get path */
    if (!(path = json_object_get(json_object_get(channel, "target"), "path"))) {
        fprintf(stderr, "Error: gltf: anim: channel: missing path\n");
        return 0;
    }
    if (!(pathStr = json_string_value(path))) {
        fprintf(stderr, "Error: gltf: anim: channel: invalid path\n");
        return 0;
    }

    if (!strcmp(pathStr, "translation")) {
        if (!track_load_vecn(context, anim, TRACK_POS, in, out, interp)) return 0;
    } else if (!strcmp(pathStr, "scale")) {
        if (!track_load_vecn(context, anim, TRACK_SCALE, in, out, interp)) return 0;
    } else if (!strcmp(pathStr, "rotation")) {
        if (!track_load_vecn(context, anim, TRACK_QUAT, in, out, interp)) return 0;
    }
    return 1;
}

static struct Animation* find_anim(struct Clip* clip, struct Node* node) {
    unsigned int i;

    for (i = 0; i < clip->numAnimations; i++) {
        if (clip->animations[i].targetNode == node) return clip->animations + i;
    }
    return NULL;
}

static struct Animation* get_anim(struct Clip* clip, struct Node* node) {
    struct Animation* res;

    if ((res = find_anim(clip, node))) {
        return res;
    } else {
        unsigned int idx;
        idx = clip->numAnimations;
        if (!anim_clip_new_anim(clip, node)) return NULL;
        /* Warning: this pointer will only be valid until the next call to clip_new_anim (realloc) */
        return clip->animations + idx;
    }
}

static int parse_anims(struct GltfContext* context, json_t* channels, json_t* samplers, struct Clip* clip) {
    unsigned int idx;
    json_t* curAnim;

    json_array_foreach(channels, idx, curAnim) {
        unsigned int nodeIdx;
        struct Animation* newAnim;
        struct Node* node;
        json_t *tgt, *jnode;

        if (!(tgt = json_object_get(curAnim, "target"))) {
            fprintf(stderr, "Error: gltf: anim: missing target\n");
            return 0;
        }
        if (!(jnode = json_object_get(tgt, "node")) || !json_is_integer(jnode)) {
            fprintf(stderr, "Error: gltf: anim: missing node target\n");
            return 0;
        }
        nodeIdx = json_integer_value(jnode);
        if (nodeIdx >= context->numNodes) {
            fprintf(stderr, "Error: gltf: anim: invalid target node index\n");
            return 0;
        }
        node = context->nodes[nodeIdx];

        /* Get the anim targetting this node or create a new one if it doesn't exist yet */
        if (!(newAnim = get_anim(clip, node))) {
            fprintf(stderr, "Error: gltf: anim: can't create new anim\n");
            return 0;
        }
        if (!parse_track(context, newAnim, curAnim, samplers)) return 0;
    }
    return 1;
}

int gltf_parse_anims(struct GltfContext* context, json_t* jroot) {
    json_t *clips, *tmp;
    unsigned int numClips, clipIdx;

    if (!(clips = json_object_get(jroot, "animations"))) return 1;
    if (!(numClips = json_array_size(clips))) return 1;

    if (!(context->clips = calloc(numClips, sizeof(context->clips)))) {
        fprintf(stderr, "Error: gltf: anim: can't allocate clips\n");
        return 0;
    }
    context->numClips = numClips;

    json_array_foreach(clips, clipIdx, tmp) {
        json_t *channels, *samplers, *name;
        const char* namestr = NULL;

        if (       !(channels = json_object_get(tmp, "channels"))
                || !(samplers = json_object_get(tmp, "samplers"))) {
            fprintf(stderr, "Warning: gltf: anim: missing channels or samplers, skipping clip\n");
            continue;
        }
        if ((name = json_object_get(tmp, "name"))) {
            namestr = json_string_value(name);
        }
        if (!(context->clips[clipIdx] = malloc(sizeof(struct Clip)))) {
            fprintf(stderr, "Error: gltf: clip: can't allocate clip\n");
            return 0;
        }
        anim_clip_init(context->clips[clipIdx]);
        if (!(context->clips[clipIdx]->name = malloc(strlen(namestr) + 1))) {
            fprintf(stderr, "Error: gltf: clip: can't allocate clip name\n");
            return 0;
        }
        memcpy(context->clips[clipIdx]->name, namestr, strlen(namestr) + 1);

        if (!parse_anims(context, channels, samplers, context->clips[clipIdx])) return 0;

        if (context->metadata && !import_add_metadata_clip(context->metadata, context->clips[clipIdx])) {
            fprintf(stderr, "Error: gltf: clip: can't add clip to metadata\n");
            return 0;
        }
        infer_clip_duration(context->clips[clipIdx]);
    }
    return 1;
}
