#include <stdlib.h>
#include <string.h>
#include <game/scene/skin.h>
#include "context.h"
#include "material.h"
#include "geometry_object.h"

static void skin_free_(void* p) {
    skin_free(p);
}

static void ogex_init_object_list(struct OgexObjectList* list) {
    list->objects = NULL;
    list->structures = NULL;
    list->num = 0;
}

static void ogex_free_object_list(struct OgexObjectList* list, void (*free_obj)(void*)) {
    unsigned int i;
    if (free_obj) {
        for (i = 0; i < list->num; i++) {
            free_obj(list->objects[i]);
        }
    }
    free(list->objects);
    free(list->structures);
}

void ogex_init_context(struct OgexContext* context) {
    context->scale = 1.0;
    context->angle = 1.0;
    context->time = 1.0;
    context->up = AXIS_Z;
    context->forward = AXIS_X;
    context->path = NULL;

    context->metadata = NULL;

    ogex_init_object_list(&context->cameras);
    ogex_init_object_list(&context->matParams);
    ogex_init_object_list(&context->geometries);
    ogex_init_object_list(&context->lights);
    ogex_init_object_list(&context->boneNodes);
    ogex_init_object_list(&context->skins);
    context->clips = NULL;
    context->numClips = 0;
}

void ogex_free_context(struct OgexContext* context) {
    unsigned int i;
    ogex_free_object_list(&context->cameras, free);
    ogex_free_object_list(&context->matParams, ogex_free_material);
    ogex_free_object_list(&context->geometries, ogex_free_geometry);
    ogex_free_object_list(&context->lights, free);
    ogex_free_object_list(&context->boneNodes, NULL);
    ogex_free_object_list(&context->skins, skin_free_);
    for (i = 0; i < context->numClips; i++) {
        if (context->clips[i]) {
            anim_free_clip(context->clips[i]);
            free(context->clips[i]);
        }
    }
    free(context->clips);
}

void ogex_swap_yz_vec(Vec3 vec) {
    float tmp;
    tmp = vec[1];
    vec[1] = vec[2];
    vec[2] = -tmp;
}

void ogex_swap_yz_mat(Mat4 mat) {
    Vec4 tmp;
    unsigned int i;

    /* Swap columns 1 and 2 */
    memcpy(tmp, mat[1], sizeof(Vec4));
    memcpy(mat[1], mat[2], sizeof(Vec4));
    memcpy(mat[2], tmp, sizeof(Vec4));
    /* Swap rows 1 and 2 */
    for (i = 0; i < 4; i++) {
        float tmpf = mat[i][1];
        mat[i][1] = mat[i][2];
        mat[i][2] = tmpf;
    }
    /* Negate relevant coefs */
    mat[0][2] *= -1.0;
    mat[1][2] *= -1.0;
    mat[2][0] *= -1.0;
    mat[2][1] *= -1.0;
    mat[3][2] *= -1.0;
}

int ogex_context_add_object(struct OgexObjectList* list, const struct ODDLStructure* ref, void* p) {
    void** tmp;
    const struct ODDLStructure** tmpref;

    if (list->num >= ((unsigned int)-1)
     || (list->num + 1) >= ((unsigned int)-1) / sizeof(void*)
     || !(tmp = realloc(list->objects, (list->num + 1) * sizeof(void*)))) return 0;
    list->objects = tmp;
    if (!(tmpref = realloc(list->structures, (list->num + 1) * sizeof(*list->structures)))) return 0;
    list->structures = tmpref;
    list->objects[list->num] = p;
    list->structures[list->num++] = ref;
    return 1;
}

void* ogex_context_find_object(const struct OgexObjectList* list, const struct ODDLStructure* ref) {
    unsigned int i;
    for (i = 0; i < list->num; i++) {
        if (list->structures[i] == ref) return list->objects[i];
    }
    return NULL;
}
