#include <game/scene/node.h>
#include <game/animation/animation.h>

#ifndef GAME_IMPORT_H
#define GAME_IMPORT_H

struct SharedData {
    struct Material** mats;
    struct VertexArray** va;
    struct Camera** cams;
    struct PointLight** plights;
    struct DirectionalLight** dlights;

    unsigned int nbMat, nbVA, nbCams, nbPLights, nbDLights;
};

struct ImportMetadata {
    struct Node** cameraNodes;
    struct Node** lightNodes;
    struct Clip* clips;

    unsigned int nbCameraNodes;
    unsigned int nbLightNodes;
    unsigned int nbClips;
};

int import_add_shared_item(void* ptr, unsigned int* size, void* newItem);

void import_free_shared_data(struct SharedData* shared);
void import_free_metadata(struct ImportMetadata* metadata);

#endif
