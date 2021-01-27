#include <stdio.h>
#include <sfnt/font_directory.h>
#include <sfnt/table/cmap.h>
#include <sfnt/table/head.h>
#include <sfnt/table/hhea.h>
#include <sfnt/table/hmtx.h>
#include <sfnt/table/maxp.h>
#include <sfnt/table/truetype/glyf.h>
#include <sfnt/table/truetype/loca.h>
#include <3dmr/math/outline.h>
#include <3dmr/font/text.h>

#ifndef TDMR_FONT_TTF_H
#define TDMR_FONT_TTF_H

struct TTF {
    struct SFNT_TableHead head;
    struct SFNT_TableCmap cmap;
    struct SFNT_TableLoca loca;
    struct SFNT_TableHhea hhea;
    struct SFNT_TableHmtx hmtx;
    struct SFNT_TableMaxp maxp;
    struct SFNT_FontDirectory fontdir;
    const struct SFNT_FontDirTblRecord* glyf;
    const union SFNT_CmapEncodingTable* cmaptbl;
    FILE* file;
};

int ttf_load(const char* ttfpath, struct TTF* ttf);
void ttf_free(struct TTF* ttf);

int ttf_glyph_outline(const struct SFNT_Glyf* glyph, struct Outline* outline);
int ttf_load_char(const struct TTF* ttf, unsigned long codepoint, struct Character* c);

#endif
