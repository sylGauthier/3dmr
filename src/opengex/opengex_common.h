#include <liboddl/liboddl.h>
#include <game/scene/scene.h>
#include <game/render/camera.h>
#include <game/material/phong.h>
#include <game/scene/opengex.h>
#include <game/scene/skin.h>

#define OGEX_MAX_NB_CLIP 32

#ifndef OGEX_COMMON_H
#define OGEX_COMMON_H

enum Axis {
    AXIS_X = 1,
    AXIS_MINUS_X,
    AXIS_Y,
    AXIS_MINUS_Y,
    AXIS_Z,
    AXIS_MINUS_Z
};

enum OgexIdentifier {
    OGEX_NONE,
    OGEX_ANIMATION,
    OGEX_ATTEN,
    OGEX_BONE_COUNT_ARRAY,
    OGEX_BONE_INDEX_ARRAY,
    OGEX_BONE_NODE,
    OGEX_BONE_REF_ARRAY,
    OGEX_BONE_WEIGHT_ARRAY,
    OGEX_CAMERA_NODE,
    OGEX_CAMERA_OBJECT,
    OGEX_CLIP,
    OGEX_COLOR,
    OGEX_GEOMETRY_NODE,
    OGEX_GEOMETRY_OBJECT,
    OGEX_INDEX_ARRAY,
    OGEX_KEY,
    OGEX_LIGHT_NODE,
    OGEX_MATERIAL,
    OGEX_MATERIAL_REF,
    OGEX_MESH,
    OGEX_METRIC,
    OGEX_MORPH,
    OGEX_NAME,
    OGEX_NODE,
    OGEX_OBJECT_REF,
    OGEX_PARAM,
    OGEX_ROTATION,
    OGEX_SCALE,
    OGEX_SKELETON,
    OGEX_SKIN,
    OGEX_TEXTURE,
    OGEX_TIME,
    OGEX_TRACK,
    OGEX_TRANSFORM,
    OGEX_TRANSLATION,
    OGEX_UNKNOWN,
    OGEX_VALUE,
    OGEX_VERTEX_ARRAY,
    OGEX_NB_IDENTIFIERS
};

enum OgexLightType {
    OGEX_LIGHT_INFINITE,
    OGEX_LIGHT_POINT,
    OGEX_LIGHT_SPOT
};

struct OgexSharedObject {
    void* object;
    struct ODDLStructure* oddlPtr;
    int persistent;
};

struct OgexSkeleton {
    struct ODDLStructure* skeleton;
    struct Skin* skin;
};

struct OgexContext {
    float scale, angle, time;
    enum Axis up, forward;
    struct ODDLDoc doc;
    const char* path;

    unsigned nbSharedObjects;
    struct OgexSharedObject* sharedObjs;

    unsigned nbSkeletons;
    struct OgexSkeleton* skeletons;

    struct Clip* clips[OGEX_MAX_NB_CLIP];

    struct Node* root;
    struct ImportMetadata* metadata;
    struct SharedData* shared;
};

int extract_scale(Vec3 scale, Mat4 t);
void swap_yz(Mat4 mat);

enum OgexIdentifier ogex_get_identifier(struct ODDLStructure* st);
int ogex_add_shared_object(struct OgexContext* context, struct ODDLStructure* oddlStruct, void* object, int persistent);
int ogex_add_skeleton(struct OgexContext* context, struct ODDLStructure* skeleton, struct Skin* skin);
void* ogex_get_shared_object(struct OgexContext* context, struct ODDLStructure* oddlStruct);
void ogex_free_context(struct OgexContext* context);

char* ogex_parse_name(struct ODDLStructure* cur);
int ogex_parse_ref(struct ODDLStructure* cur, struct ODDLStructure** res);
int ogex_parse_param(struct ODDLStructure* cur, char** attrib, float* value);
int ogex_parse_color(struct ODDLStructure* cur, char** attrib, Vec3 color);
int ogex_parse_material(struct OgexContext* context, struct ODDLStructure* cur);
int ogex_parse_geometry_object(struct OgexContext* context, struct ODDLStructure* cur);
int ogex_parse_camera_object(struct OgexContext* context, struct ODDLStructure* cur);
int ogex_parse_light_object(struct OgexContext* context, struct ODDLStructure* cur);
int ogex_parse_transform(struct OgexContext* context, struct ODDLStructure* cur, Mat4 transform);
int ogex_parse_transforms(struct OgexContext* context, struct ODDLStructure* cur, Mat4** transforms, unsigned int* nbTransforms);

int ogex_parse_node(struct OgexContext* context, struct Node* root, struct ODDLStructure* cur);
int ogex_parse_geometry_node(struct OgexContext* context, struct Node* node, struct ODDLStructure* cur);
int ogex_parse_camera_node(struct OgexContext* context, struct Node* node, struct ODDLStructure* cur);
int ogex_parse_light_node(struct OgexContext* context, struct Node* newNode, struct ODDLStructure* cur);
int ogex_parse_skin(struct OgexContext* context, struct Skin* skin, struct ODDLStructure* cur);
int ogex_parse_animation(struct OgexContext* context, struct Node* node, struct ODDLStructure* cur);
int ogex_parse_clip(struct OgexContext* context, struct ODDLStructure* cur);

int ogex_post_parse_skeletons(struct OgexContext* context);

#endif
