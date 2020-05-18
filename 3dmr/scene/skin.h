#ifndef TDMR_SCENE_SKIN_H
#define TDMR_SCENE_SKIN_H

#include <3dmr/scene/node.h>
#include <3dmr/render/uniform_buffer.h>

#define MAX_NB_BONES 100

struct Skin {
    Mat4* bindPose;
    Mat4 skinTransform;

    struct Node** bones;
    unsigned int numBones;
    struct Node* root;

    struct UniformBuffer transforms;
};

int skin_gen(struct Skin* skin, unsigned int numBones);
struct Skin* skin_new(unsigned int numBones);

void skin_update_node(struct Skin* skin, unsigned int boneIdx);
void skin_load(struct Skin* skin);
void skin_del(struct Skin* skin);
void skin_free(struct Skin* skin);

void vertex_array_set_skin(struct VertexArray* va, const struct Skin* skin);

#endif
