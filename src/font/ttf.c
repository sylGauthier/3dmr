#include <stdlib.h>
#include <string.h>
#include <sfnt/character_map.h>
#include <sfnt/platform.h>
#include <3dmr/font/ttf.h>
#include <3dmr/math/linear_algebra.h>

static int cmap_tbl_score(const struct SFNT_CmapEncodingRecord* rcd, const union SFNT_CmapEncodingTable* tbl) {
    /*                  0   1   2   3  4   5  6   7   8   9 10  11 12 13 */
    int fmtscore[14] = {0, -1, -1, -1, 3, -1, 2, -1, -1, -1, 4, -1, 5, 4};
    int score = -1;
    switch (rcd->platformId) {
        case SFNT_PLATFORM_UNICODE:
            if (rcd->encodingId < 7U && rcd->encodingId != SFNT_UNICODE_ENCODING_UNICODE_VARIATION_SEQ) {
                score = rcd->encodingId;
            }
            break;
        case SFNT_PLATFORM_WINDOWS:
            switch (rcd->encodingId) {
                case SFNT_WINDOWS_ENCODING_UNICODE:
                    score = SFNT_UNICODE_ENCODING_UNICODE;
                    break;
                case SFNT_WINDOWS_ENCODING_UNICODE_BMP:
                    score = SFNT_UNICODE_ENCODING_UNICODE_2_0_BMP;
                    break;
            }
            break;
    }
    if (tbl->format >= sizeof(fmtscore) / sizeof(*fmtscore)) score = -1;
    if (score >= 0) {
        if (fmtscore[tbl->format] < 0) {
            score = fmtscore[tbl->format];
        } else {
            score = 8 * score + fmtscore[tbl->format];
        }
    }
    return score;
}

const union SFNT_CmapEncodingTable* cmap_unicode_find_best_subtable(const struct SFNT_TableCmap* cmap) {
    union SFNT_CmapEncodingTable* best = NULL;
    int score, bestScore = 0;
    unsigned int i;

    for (i = 0; i < cmap->numTables; i++) {
        if (bestScore <= (score = cmap_tbl_score(cmap->encodingRecord + i, cmap->encodingTable + i))) {
            bestScore = score;
            best = cmap->encodingTable + i;
        }
    }
    return best;
}

int ttf_load(const char* ttfpath, struct TTF* ttf) {
    int hasftdir = 0, hascmap = 0, hasloca = 0, hashmtx = 0;

    if (!(ttf->file = fopen(ttfpath, "rb"))) {
        perror("Error: failed to open font file");
    } else if (!(hasftdir = sfnt_read_font_directory(ttf->file, &ttf->fontdir))) {
        fputs("Error: failed to read font directory\n", stderr);
    } else if (!sfnt_read_head(&ttf->fontdir, ttf->file, &ttf->head)) {
        fputs("Error: failed to read font head table\n", stderr);
    } else if (!(hascmap = sfnt_read_cmap(&ttf->fontdir, ttf->file, &ttf->cmap))) {
        fputs("Error: failed to read font cmap table\n", stderr);
    } else if (!(ttf->cmaptbl = cmap_unicode_find_best_subtable(&ttf->cmap))) {
        fputs("Error: failed to find unicode cmap subtable\n", stderr);
    } else if (!(hasloca = sfnt_read_loca(&ttf->fontdir, ttf->file, ttf->head.indexToLocFormat, &ttf->loca))) {
        fputs("Error: failed to read font loca table\n", stderr);
    } else if (!sfnt_read_maxp(&ttf->fontdir, ttf->file, &ttf->maxp)) {
        fputs("Error: failed to read font maxp table\n", stderr);
    } else if (!sfnt_read_hhea(&ttf->fontdir, ttf->file, &ttf->hhea)) {
        fputs("Error: failed to read font hhea table\n", stderr);
    } else if (!(hashmtx = sfnt_read_hmtx(&ttf->fontdir, ttf->file, ttf->hhea.numHMetrics, ttf->maxp.numGlyphs, &ttf->hmtx))) {
        fputs("Error: failed to read font hmtx table\n", stderr);
    } else if (!(ttf->glyf = sfnt_find_table(&ttf->fontdir, SFNT_TAG_GLYF))) {
        fputs("Error: failed to find font glyf table\n", stderr);
    } else {
        return 1;
    }
    if (hasftdir) sfnt_free_font_directory(&ttf->fontdir);
    if (hascmap) sfnt_free_cmap(&ttf->cmap);
    if (hasloca) sfnt_free_loca(&ttf->loca);
    if (hashmtx) sfnt_free_hmtx(&ttf->hmtx);
    if (ttf->file) fclose(ttf->file);
    return 0;
}

void ttf_free(struct TTF* ttf) {
    sfnt_free_font_directory(&ttf->fontdir);
    sfnt_free_cmap(&ttf->cmap);
    sfnt_free_loca(&ttf->loca);
    sfnt_free_hmtx(&ttf->hmtx);
    fclose(ttf->file);
}

static void pt_to_vec2(const struct SFNT_GlyfPoint* pt, Vec2 vec2) {
    vec2[0] = pt->x;
    vec2[1] = pt->y;
}

static void pts_middle_to_vec2(const struct SFNT_GlyfPoint* pt1, const struct SFNT_GlyfPoint* pt2, Vec2 vec2) {
    vec2[0] = (pt1->x + pt2->x) * 0.5f;
    vec2[1] = (pt1->y + pt2->y) * 0.5f;
}

int ttf_glyph_outline(const struct SFNT_Glyf* glyph, struct Outline* outline) {
    const struct SFNT_GlyfPoint* pts;
    union OutlineSegment* segment;
    uint32_t start, end, cur, last, last2;
    int16_t i;
    int ok = 1, laston, curon;

    if (glyph->numContours < 0) return 0; /* composite glyph; not supported ATM */
    if (glyph->numContours == 0) {
        outline->contours = NULL;
        outline->numContours = 0;
        return 1;
    }
    if (((unsigned int)glyph->numContours) >= ((size_t)-1) / sizeof(*outline->contours) || !(outline->contours = malloc(((unsigned int)glyph->numContours) * sizeof(*outline->contours)))) {
        return 0;
    }
    pts = glyph->data.simple.points;
    outline->numContours = glyph->numContours;
    for (i = 0, start = 0; ok && i < glyph->numContours; i++, start = end + 1U) {
        end = glyph->data.simple.endPoints[i];
        outline->contours[i].segments = NULL;
        outline->contours[i].numSegments = 0;
        for (cur = start, laston = (pts[end].flags & SFNT_GLYF_ON_CURVE_POINT); ok && cur <= end; cur++, laston = curon) {
            curon = (pts[cur].flags & SFNT_GLYF_ON_CURVE_POINT);
            switch (2 * laston + curon) {
                case 0: /* off-off: implicit on-pt in between, that closes current bezier segment */
                case SFNT_GLYF_ON_CURVE_POINT: /* off-on: end of a bezier segment */
                case 2 * SFNT_GLYF_ON_CURVE_POINT + SFNT_GLYF_ON_CURVE_POINT: /* on-on: end of a linear segment */
                    if (!++outline->contours[i].numSegments) ok = 0;
                    break;
                case 2 * SFNT_GLYF_ON_CURVE_POINT: /* on-off: start of a bezier segment */
                    break;
            }
        }
        ok = ok && (outline->contours[i].numSegments < ((size_t)-1) / sizeof(*outline->contours[i].segments))
            && (outline->contours[i].segments = malloc(outline->contours[i].numSegments * sizeof(*outline->contours[i].segments)));
        segment = outline->contours[i].segments;
        for (cur = start, laston = (pts[end].flags & SFNT_GLYF_ON_CURVE_POINT); ok && cur <= end; cur++, laston = curon) {
            curon = (pts[cur].flags & SFNT_GLYF_ON_CURVE_POINT);
            switch (2 * laston + curon) {
                case 0: /* off-off: implicit on-pt in between, that closes current bezier segment */
                    segment->quadratic.type = OUTLINE_SEGMENT_QUADRATIC;
                    last = (cur > start) ? (cur - 1U) : end;
                    pts_middle_to_vec2(pts + cur, pts + last, segment->quadratic.p2);
                    pt_to_vec2(pts + last, segment->quadratic.p1);
                    last2 = (last > start) ? (last - 1U) : end;
                    if (pts[last2].flags & SFNT_GLYF_ON_CURVE_POINT) {
                        pt_to_vec2(pts + last2, segment->quadratic.p0);
                    } else {
                        pts_middle_to_vec2(pts + last, pts + last2, segment->quadratic.p0);
                    }
                    segment++;
                    break;
                case SFNT_GLYF_ON_CURVE_POINT: /* off-on: end of a bezier segment */
                    segment->quadratic.type = OUTLINE_SEGMENT_QUADRATIC;
                    last = (cur > start) ? (cur - 1U) : end;
                    pt_to_vec2(pts + cur, segment->quadratic.p2);
                    pt_to_vec2(pts + last, segment->quadratic.p1);
                    last2 = (last > start) ? (last - 1U) : end;
                    if (pts[last2].flags & SFNT_GLYF_ON_CURVE_POINT) {
                        pt_to_vec2(pts + last2, segment->quadratic.p0);
                    } else {
                        pts_middle_to_vec2(pts + last, pts + last2, segment->quadratic.p0);
                    }
                    segment++;
                    break;
                case 2 * SFNT_GLYF_ON_CURVE_POINT + SFNT_GLYF_ON_CURVE_POINT: /* on-on: end of a linear segment */
                    segment->linear.type = OUTLINE_SEGMENT_LINEAR;
                    last = (cur > start) ? (cur - 1U) : end;
                    pt_to_vec2(pts + cur, segment->linear.p1);
                    pt_to_vec2(pts + last, segment->linear.p0);
                    segment++;
                    break;
                case 2 * SFNT_GLYF_ON_CURVE_POINT: /* on-off: start of a bezier segment */
                    break;
            }
        }
    }
    if (!ok) {
        do free(outline->contours[i--].segments); while (i);
        free(outline->contours);
        return 0;
    }
    return 1;
}

int ttf_load_char(const struct TTF* ttf, unsigned long codepoint, struct Character* c) {
    struct SFNT_Glyf glyph;
    struct SFNT_TableHmtxHMetric hmetric;
    unsigned long glyphid = sfnt_cmap_codepoint_to_glyphid(ttf->cmaptbl, codepoint);
    uint32_t offset = sfnt_loca_index(&ttf->loca, glyphid);
    uint32_t offsetEnd = sfnt_loca_index(&ttf->loca, glyphid + 1U);
    int hasglyph = 0, hasoutline = 0;

    if (!sfnt_get_hmetric(&ttf->hmtx, glyphid, &hmetric)) {
        fputs("Error: failed to get horizontal metrics\n", stderr);
    } else if (offsetEnd <= offset) {
        c->outline.contours = NULL;
        c->outline.numContours = 0;
        c->xMin = c->xMax = c->yMin = c->yMax = 0;
        hasoutline = 1;
    } else if (!sfnt_seek(ttf->glyf, offset, ttf->file)) {
        fputs("Error: failed to find glyph\n", stderr);
    } else if (!(hasglyph = sfnt_read_glyf(ttf->file, &glyph))) {
        fputs("Error: failed to read glyph\n", stderr);
    } else if (glyph.numContours < 0) {
        fputs("Error: composite glyphs are not supported\n", stderr);
    } else if (!(hasoutline = ttf_glyph_outline(&glyph, &c->outline))) {
        fputs("Error: failed to compute outline\n", stderr);
    } else {
        c->xMin = glyph.xMin;
        c->xMax = glyph.xMax;
        c->yMin = glyph.yMin;
        c->yMax = glyph.yMax;
    }
    if (hasoutline) {
        c->advance = hmetric.advanceWidth;
        c->lsb = hmetric.leftSideBearing;
        if (hasglyph) sfnt_free_glyf(&glyph);
        return 1;
    }
    if (hasglyph) sfnt_free_glyf(&glyph);
    return 0;
}

int ttf_load_chars(const struct TTF* ttf, const char* str, struct Character** c, size_t* numChars) {
    struct Character* dest;
    struct Character* res;
    unsigned int l;

    l = strlen(str);
    if (!(dest = res = malloc(l * sizeof(struct Character)))) {
        fprintf(stderr, "Error: can't allocate memory for Characters\n");
    } else {
        unsigned long cp;
        unsigned int n;
        unsigned char ch;

        while ((cp = (unsigned char)*str++)) {
            for (n = 0; n < 4U && (cp & (0x80U >> n)); n++);
            if (n >= 4U) {
                while (dest > res) character_free(--dest);
                free(res);
                return 0;
            }
            cp &= (0x3FU >> n) | (!n << 6U);
            while (n-- > 1U && (ch = *str++)) cp = (cp << 6U) | (ch & 0x3FU);
            if (!ttf_load_char(ttf, cp, dest)) {
                while (dest > res) character_free(--dest);
                free(res);
                return 0;
            }
            dest++;
        }
        *c = res;
        *numChars = dest - res;
        return 1;
    }
    return 0;
}
