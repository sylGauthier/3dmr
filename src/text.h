#ifndef TEXT_H
#define TEXT_H

#include "mesh/mesh.h"
#include "geometry/geometry.h"

#include <ft2build.h>
#include FT_FREETYPE_H

struct BitmapGlyph {
    uint32_t char_code;
    float texture[4];
    float bitmap_width;
    float bitmap_height;
    float bitmap_top;
    float bitmap_left;
    float advance_x;
    float advance_y;
};

struct BitmapFont {
    const char *path;
    const char *name;
    int size;
    int nb_glyphs;
    struct BitmapGlyph *glyphs;
    int atlas_width;
    int atlas_height;
    GLuint texture_atlas;
    FT_Face face;
};

/* Create a simple monospace bitmap font, support only printable ascii */
struct BitmapFont *simple_bitmap_font(const int size);

struct BitmapFont *ttf_bitmap_font(const char *path, const int font_height);

void font_free(struct BitmapFont *font);

int new_text(struct BitmapFont *font, const char *text, struct Mesh *mesh);

#endif
