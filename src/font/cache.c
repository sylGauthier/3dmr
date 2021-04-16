#include <stdlib.h>
#include <3dmr/font/cache.h>

void character_cache_init(struct CharacterCache* cache) {
    struct CharacterCache zero = {0};
    *cache = zero;
}

void character_cache_free(struct CharacterCache* cache) {
    while (cache->numEntries) character_free(&cache->entries[--(cache->numEntries)].character);
    free(cache->entries);
}

static int entry_cmp(const void* a, const void* b) {
    const struct CharacterCacheEntry* ea = a;
    const struct CharacterCacheEntry* eb = b;
    return (ea->codepoint > eb->codepoint) - (ea->codepoint < eb->codepoint);
}

int character_cache_insert(struct CharacterCache* cache, unsigned long codepoint, const struct Character* c) {
    struct CharacterCacheEntry key, *new;
    size_t a, b, m, n = cache->numEntries;
    int r;

    key.codepoint = codepoint;
    if (!n || entry_cmp(&key, cache->entries) < 0) {
        a = 0;
    } else if (entry_cmp(&key, cache->entries + n - 1U) > 0) {
        a = n;
    } else {
        for (a = 0, b = n - 1; a <= b; ) {
            m = (a >> 1U) + (b >> 1U) + (((a & 1U) + (b & 1U)) >> 1U);
            r = entry_cmp(&key, cache->entries + m);
            if (r < 0) {
                b = m - 1U;
            } else if (r > 0) {
                a = m + 1U;
            } else {
                character_free(&cache->entries[m].character);
                cache->entries[m].character = *c;
                return 1;
            }
        }
        a = m + ((unsigned int)(r > 0));
    }
    if (!++n || n >= ((size_t)-1) / sizeof(*cache->entries)
     || !(new = realloc(cache->entries, n * sizeof(*cache->entries)))) {
        return 0;
    }
    cache->entries = new;
    cache->numEntries = n;
    for (b = n - 1U; b > a; b--) {
        new[b] = new[b - 1U];
    }
    new[a].character = *c;
    new[a].codepoint = codepoint;
    return 1;
}

int character_cache_get(const struct CharacterCache* cache, unsigned long codepoint, struct Character* c) {
    struct CharacterCacheEntry key, *found;
    key.codepoint = codepoint;
    if ((found = bsearch(&key, cache->entries, cache->numEntries, sizeof(key), entry_cmp))) {
        *c = found->character;
        return 1;
    }
    return 0;
}
