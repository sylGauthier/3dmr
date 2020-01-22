#include <stdio.h>
#include <stdlib.h>

#include <game/scene/skin.h>
#include <game/render/shader.h>

static void compute_transforms(struct Skin* skin) {
    unsigned int i;
    Mat4 invBindPose, invSkin;
    Mat4 tmp, tmp2;
    Mat4* transforms = skin->transforms.cache;

    invert4m(invSkin, MAT_CONST_CAST(skin->skinTransform));
    for (i = 0; i < skin->numBones; i++) {
        invert4m(invBindPose, MAT_CONST_CAST(skin->bindPose[i]));
        mul4mm(tmp, MAT_CONST_CAST(skin->bones[i]->model), MAT_CONST_CAST(invBindPose));
        mul4mm(tmp2, MAT_CONST_CAST(tmp), MAT_CONST_CAST(skin->skinTransform));
        mul4mm(transforms[i], MAT_CONST_CAST(invSkin), MAT_CONST_CAST(tmp2));
    }
    uniform_buffer_invalidate(&skin->transforms, 0, skin->numBones * sizeof(Mat4));
}

int skin_gen(struct Skin* skin, unsigned int numBones) {
    unsigned int uboSize = numBones * sizeof(Mat4);

    if (!(skin->bindPose = malloc(numBones * sizeof(*skin->bindPose)))
     || !(skin->bones = malloc(numBones * sizeof(*skin->bones)))
     || !uniform_buffer_gen(uboSize, &skin->transforms)) {
        free(skin->bindPose);
        free(skin->bones);
        return 0;
    }
    skin->numBones = numBones;
    return 1;
}

struct Skin* skin_new(unsigned int numBones) {
    struct Skin* skin;
    if ((skin = malloc(sizeof(*skin)))) {
        if (skin_gen(skin, numBones)) {
            return skin;
        }
        free(skin);
    }
    return NULL;
}

void skin_load(struct Skin* skin) {
    /* TODO: be smarter and only recompute bones that were modified */
    compute_transforms(skin);
    uniform_buffer_send(&skin->transforms);
    glBindBufferRange(GL_UNIFORM_BUFFER, BONES_UBO_BINDING, skin->transforms.ubo, 0, skin->numBones * sizeof(Mat4));
}

void skin_del(struct Skin* skin) {
    free(skin->bones);
    free(skin->bindPose);
    uniform_buffer_del(&skin->transforms);
}

void skin_free(struct Skin* skin) {
    if (skin) {
        skin_del(skin);
        free(skin);
    }
}
