#ifndef SKIN_H
#define SKIN_H

#include <game/scene/node.h>
#include <game/render/uniform_buffer.h>

#define MAX_NB_BONES 100

struct Skin {
    struct Node** bones;
    Mat4* bindPose;
    Mat4 skinTransform;

    struct UniformBuffer* transforms;
    unsigned int nbBones;

    unsigned int* indexArray;
    float* weightArray;
    unsigned int nbVertices;
};

int skin_gen(struct Skin* skin);
void skin_update_node(struct Skin* skin, unsigned int boneIdx);
void skin_load(struct Skin* skin);
void skin_free(struct Skin* skin);

#endif
