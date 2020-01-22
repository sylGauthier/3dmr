#ifndef SKIN_H
#define SKIN_H

#include <game/scene/node.h>
#include <game/render/uniform_buffer.h>

#define MAX_NB_BONES 100

struct Skin {
    Mat4* bindPose;
    Mat4 skinTransform;

    struct Node** bones;
    unsigned int numBones;

    struct UniformBuffer transforms;
};

int skin_gen(struct Skin* skin, unsigned int numBones);
struct Skin* skin_new(unsigned int numBones);

void skin_update_node(struct Skin* skin, unsigned int boneIdx);
void skin_load(struct Skin* skin);
void skin_del(struct Skin* skin);
void skin_free(struct Skin* skin);

#endif
