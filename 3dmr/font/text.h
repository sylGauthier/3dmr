#include <3dmr/math/outline.h>
#include <3dmr/render/texture.h>

#ifndef TDMR_FONT_TEXT_H
#define TDMR_FONT_TEXT_H

struct Character {
    struct Outline outline;
    float xMin, xMax, yMin, yMax;
    float advance, lsb;
};

void character_free(struct Character* c);

unsigned char* text_to_sdm_buffer(const struct Character* chars, size_t numChars, size_t mapHeight, size_t* mapWidth, int yMinMaxProvided, int yMin, int yMax);

GLuint text_to_sdm_texture(const struct Character* chars, size_t numChars, size_t mapHeight, size_t* mapWidth, int yMinMaxProvided, int yMin, int yMax);

#endif
