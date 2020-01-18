#include <stdio.h>
#include <stdlib.h>

#include <game/scene/skin.h>
#include <game/render/shader.h>

static int check_skeleton(struct Skin* skin) {
    unsigned int i;

    if (skin->nbBones > MAX_NB_BONES) {
        fprintf(stderr, "Error: Skin has too many bones, max allowed is %d\n", skin->nbBones);
        return 0;
    }

    for (i = 0; i < 2 * skin->nbBones; i++) {
        unsigned int idx = skin->indexArray[i];

        if (idx >= skin->nbBones) {
            fprintf(stderr, "Error: check_skeleton: index greater than number of bones\n");
            return 0;
        }
    }
    return 1;
}

static void compute_transforms(struct Skin* skin, Mat4* transforms) {
    unsigned int i;
    Mat4 invBindPose, invSkin;
    Mat4 tmp, tmp2;

    invert4m(invSkin, MAT_CONST_CAST(skin->skinTransform));
    for (i = 0; i < skin->nbBones; i++) {
        invert4m(invBindPose, MAT_CONST_CAST(skin->bindPose[i]));
        mul4mm(tmp, MAT_CONST_CAST(skin->bones[i]->model), MAT_CONST_CAST(invBindPose));
        mul4mm(tmp2, MAT_CONST_CAST(tmp), MAT_CONST_CAST(skin->skinTransform));
        mul4mm(transforms[i], MAT_CONST_CAST(invSkin), MAT_CONST_CAST(tmp2));
    }
}

int skin_gen(struct Skin* skin) {
    unsigned int uboSize = skin->nbBones * sizeof(Mat4);

    if (!check_skeleton(skin)) {
        fprintf(stderr, "Error: skin_gen_ubo: invalid skeleton\n");
        return 0;
    }
    if (!(skin->transforms = uniform_buffer_new(uboSize))) {
        return 0;
    }
    compute_transforms(skin, (void*)skin->transforms->cache);
    uniform_buffer_invalidate_cache(skin->transforms);
    uniform_buffer_send(skin->transforms);
    return 1;
}

void skin_load(struct Skin* skin) {
    /* TODO: be smarter and only recompute/resend bones that were modified */
    compute_transforms(skin, (void*)skin->transforms->cache);
    uniform_buffer_invalidate_cache(skin->transforms);
    uniform_buffer_send(skin->transforms);
    glBindBufferRange(GL_UNIFORM_BUFFER, BONES_UBO_BINDING, skin->transforms->ubo, 0, skin->nbBones * sizeof(Mat4));
}

void skin_free(struct Skin* skin) {
    free(skin->bones);
    free(skin->bindPose);
    uniform_buffer_free(skin->transforms);

    free(skin->indexArray);
    free(skin->weightArray);
}
