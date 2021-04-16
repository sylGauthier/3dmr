#include <stddef.h>
#include "./text.h"

#ifndef TDMR_FONT_CACHE_H
#define TDMR_FONT_CACHE_H

struct CharacterCache {
    struct CharacterCacheEntry {
        struct Character character;
        unsigned long codepoint;
    } *entries;
    size_t numEntries;
};

void character_cache_init(struct CharacterCache* cache);
void character_cache_free(struct CharacterCache* cache);

int character_cache_insert(struct CharacterCache* cache, unsigned long codepoint, const struct Character* c);
int character_cache_get(const struct CharacterCache* cache, unsigned long codepoint, struct Character* c);

#endif
