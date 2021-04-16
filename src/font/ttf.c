#include <stdlib.h>
#include <string.h>
#include <sfnt/character_map.h>
#include <sfnt/font_collection.h>
#include <sfnt/platform.h>
#include <3dmr/font/cache.h>
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

static int ttf_load_file(struct TTF* ttf) {
    int hasftdir = 0, hascmap = 0, hasloca = 0, hashmtx = 0;

    if (!ttf->file) {
        fputs("Error: failed to open font file\n", stderr);
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

int ttf_load(const char* ttfpath, struct TTF* ttf) {
    ttf->file = fopen(ttfpath, "rb");
    return ttf_load_file(ttf);
}

int ttc_load(const char* ttcpath, unsigned long fnum, struct TTF* ttf) {
    if ((ttf->file = fopen(ttcpath, "rb"))) {
        struct SFNT_TTC ttc;
        int hasttc, ok;

        ok = (hasttc = sfnt_read_ttc(ttf->file, &ttc)
          && sfnt_seek_ttc(&ttc, fnum, ttf->file));
        if (hasttc) sfnt_free_ttc(&ttc);
        if (!ok) {
            fputs("Error: failed to read TTC file\n", stderr);
            fclose(ttf->file);
            return 0;
        }
    }
    return ttf_load_file(ttf);
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

struct GlyphFloat {
    struct GlyphFloatPoint {
        Vec2 coords;
        unsigned int flags;
    } *points;
    unsigned int* endPoints;
    size_t numContours;
};

static void free_glyph_float(struct GlyphFloat* g) {
    free(g->points);
    free(g->endPoints);
}

static int simple_glyph_to_float(const struct SFNT_Glyf* src, struct GlyphFloat* dest) {
    if (src->numContours < 0) {
        return 0;
    } else if (!src->numContours) {
        dest->points = NULL;
        dest->endPoints = NULL;
        dest->numContours = 0;
    } else {
        unsigned int numContours = src->numContours;
        unsigned int numPoints = src->data.simple.endPoints[numContours - 1] + 1U;
        unsigned int i;

        dest->points = NULL;
        if (numPoints > ((size_t)-1) / sizeof(*dest->points)
         || numContours > ((size_t)-1) / sizeof(*dest->endPoints)
         || !(dest->points = malloc(numPoints * sizeof(*dest->points)))
         || !(dest->endPoints = malloc(numContours * sizeof(*dest->endPoints)))) {
            free(dest->points);
            return 0;
        }
        for (i = 0; i < numPoints; i++) {
            pt_to_vec2(src->data.simple.points + i, dest->points[i].coords);
            dest->points[i].flags = src->data.simple.points[i].flags;
        }
        for (i = 0; i < numContours; i++) {
            dest->endPoints[i] = src->data.simple.endPoints[i];
        }
        dest->numContours = numContours;
    }
    return 1;
}

static int glyph_outline(const struct GlyphFloat* glyph, struct Outline* outline) {
    const struct GlyphFloatPoint* pts;
    union OutlineSegment* segment;
    uint32_t start, end, cur, last, last2;
    int16_t i;
    int ok = 1, laston, curon;

    if (glyph->numContours < 0) return 0; /* composite glyph; not handled here */
    if (glyph->numContours == 0) {
        outline->contours = NULL;
        outline->numContours = 0;
        return 1;
    }
    if (glyph->numContours >= ((size_t)-1) / sizeof(*outline->contours) || !(outline->contours = malloc(((unsigned int)glyph->numContours) * sizeof(*outline->contours)))) {
        return 0;
    }
    pts = glyph->points;
    outline->numContours = glyph->numContours;
    for (i = 0, start = 0; ok && i < glyph->numContours; i++, start = end + 1U) {
        end = glyph->endPoints[i];
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
                    add2v(segment->quadratic.p2, pts[cur].coords, pts[last].coords);
                    scale2v(segment->quadratic.p2, 0.5f);
                    memcpy(segment->quadratic.p1, pts[last].coords, sizeof(Vec2));
                    last2 = (last > start) ? (last - 1U) : end;
                    if (pts[last2].flags & SFNT_GLYF_ON_CURVE_POINT) {
                        memcpy(segment->quadratic.p0, pts[last2].coords, sizeof(Vec2));
                    } else {
                        add2v(segment->quadratic.p0, pts[last].coords, pts[last2].coords);
                        scale2v(segment->quadratic.p0, 0.5f);
                    }
                    segment++;
                    break;
                case SFNT_GLYF_ON_CURVE_POINT: /* off-on: end of a bezier segment */
                    segment->quadratic.type = OUTLINE_SEGMENT_QUADRATIC;
                    last = (cur > start) ? (cur - 1U) : end;
                    memcpy(segment->quadratic.p2, pts[cur].coords, sizeof(Vec2));
                    memcpy(segment->quadratic.p1, pts[last].coords, sizeof(Vec2));
                    last2 = (last > start) ? (last - 1U) : end;
                    if (pts[last2].flags & SFNT_GLYF_ON_CURVE_POINT) {
                        memcpy(segment->quadratic.p0, pts[last2].coords, sizeof(Vec2));
                    } else {
                        add2v(segment->quadratic.p0, pts[last].coords, pts[last2].coords);
                        scale2v(segment->quadratic.p0, 0.5f);
                    }
                    segment++;
                    break;
                case 2 * SFNT_GLYF_ON_CURVE_POINT + SFNT_GLYF_ON_CURVE_POINT: /* on-on: end of a linear segment */
                    segment->linear.type = OUTLINE_SEGMENT_LINEAR;
                    last = (cur > start) ? (cur - 1U) : end;
                    memcpy(segment->linear.p1, pts[cur].coords, sizeof(Vec2));
                    memcpy(segment->linear.p0, pts[last].coords, sizeof(Vec2));
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

int ttf_glyph_outline(const struct SFNT_Glyf* glyph, struct Outline* outline) {
    struct GlyphFloat tmp;
    int ret = 0;

    if (simple_glyph_to_float(glyph, &tmp)) {
        ret = glyph_outline(&tmp, outline);
        free_glyph_float(&tmp);
    }
    return ret;
}

struct CompositeContext {
    struct GlyphFloat glyf;
    const struct SFNT_GlyfComponent* comps;
    struct SFNT_GlyfComponent* mycomps;
    size_t compNum, numComps;
};

static int push_composite_context(struct CompositeContext** stack, size_t* numStack, const struct SFNT_GlyfComponent* comps, struct SFNT_GlyfComponent* mycomps, size_t numComps) {
    struct CompositeContext* ctx;
    size_t n;

    if ((n = *numStack) >= ((size_t)-1) || ++n > ((size_t)-1) / sizeof(**stack)
     || !(ctx = realloc(*stack, n * sizeof(**stack)))) {
        return 0;
    }
    *stack = ctx;
    ctx += (*numStack)++;
    ctx->glyf.numContours = 0;
    ctx->glyf.endPoints = NULL;
    ctx->glyf.points = NULL;
    ctx->comps = comps;
    ctx->mycomps = mycomps;
    ctx->compNum = 0;
    ctx->numComps = numComps;
    return 1;
}

static int ttf_composite_outline(const struct TTF* ttf, const struct SFNT_Glyf* g, struct Character* c) {
    struct CompositeContext* stack = NULL;
    size_t numStack = 0;
    int ok = 1;

    if (!push_composite_context(&stack, &numStack, g->data.composite.components, NULL, g->data.composite.numComponents)) return 0;
    while (ok && numStack) {
        struct CompositeContext* ctx = stack + (numStack - 1U);

        if (ctx->compNum >= ctx->numComps) {
            if (--numStack) {
                struct CompositeContext* base = stack + (numStack - 1U);
                unsigned int* endPoints = NULL;
                struct GlyphFloatPoint* points = NULL;
                size_t i, basepts, newpts, totalpts, totalctrs;

                if (base->glyf.numContours) {
                    basepts = base->glyf.endPoints[base->glyf.numContours - 1U] + 1U;
                } else {
                    basepts = 0;
                }
                if (ctx->glyf.numContours) {
                    newpts = ctx->glyf.endPoints[ctx->glyf.numContours - 1U] + 1U;
                } else {
                    newpts = 0;
                }
                ok = basepts <= ((size_t)-1) - newpts
                    && (totalpts = basepts + newpts) <= ((size_t)-1) / sizeof(*base->glyf.points)
                    && base->glyf.numContours <= ((size_t)-1) - ctx->glyf.numContours
                    && (totalctrs = base->glyf.numContours + ctx->glyf.numContours) <= ((size_t)-1) / sizeof(*base->glyf.endPoints)
                    && (endPoints = realloc(base->glyf.endPoints, totalctrs * sizeof(*base->glyf.endPoints)))
                    && (points = realloc(base->glyf.points, totalpts * sizeof(*base->glyf.points)));
                if (endPoints) base->glyf.endPoints = endPoints;
                if (points) base->glyf.points = points;
                if (ok) {
                    Mat2 scale;
                    Vec2 translation;
                    const struct SFNT_GlyfComponent* comp = base->comps + base->compNum++;

                    scale[0][0] = ((float)sfnt_f2d14_ipart(comp->transform[0][0])) + ((float)sfnt_f2d14_fpart(comp->transform[0][0])) / ((float)(0x4000U));
                    scale[0][1] = ((float)sfnt_f2d14_ipart(comp->transform[0][1])) + ((float)sfnt_f2d14_fpart(comp->transform[0][1])) / ((float)(0x4000U));
                    scale[1][0] = ((float)sfnt_f2d14_ipart(comp->transform[1][0])) + ((float)sfnt_f2d14_fpart(comp->transform[1][0])) / ((float)(0x4000U));
                    scale[1][1] = ((float)sfnt_f2d14_ipart(comp->transform[1][1])) + ((float)sfnt_f2d14_fpart(comp->transform[1][1])) / ((float)(0x4000U));
                    if (comp->flags & SFNT_GLYF_ARGS_ARE_XY_VALUES) {
                        translation[0] = comp->args[0].offset;
                        translation[1] = comp->args[1].offset;
                    } else {
                        Vec2 t;
                        if (comp->args[0].ptNum >= basepts || comp->args[1].ptNum >= newpts) {
                            ok = 0;
                            break;
                        }
                        mul2mv(t, MAT_CONST_CAST(scale), base->glyf.points[basepts + comp->args[1].ptNum].coords);
                        sub2v(translation, base->glyf.points[comp->args[0].ptNum].coords, t);
                    }
                    for (i = 0; i < newpts; i++) {
                        mul2mv(points[basepts + i].coords, MAT_CONST_CAST(scale), ctx->glyf.points[i].coords);
                        incr2v(points[basepts + i].coords, translation);
                        points[basepts + i].flags = ctx->glyf.points[i].flags;
                    }
                    for (i = 0; i < ctx->glyf.numContours; i++) {
                        endPoints[base->glyf.numContours + i] = ctx->glyf.endPoints[i] + basepts;
                    }
                    base->glyf.numContours += ctx->glyf.numContours;
                }
            } else {
                ok = glyph_outline(&ctx->glyf, &c->outline);
            }
            free_glyph_float(&ctx->glyf);
            free(ctx->mycomps);
        } else {
            struct SFNT_Glyf glyph;
            const struct SFNT_GlyfComponent* comp = ctx->comps + ctx->compNum;
            uint32_t offset = sfnt_loca_index(&ttf->loca, comp->glyphIndex);
            uint32_t offsetEnd = sfnt_loca_index(&ttf->loca, comp->glyphIndex + 1U);
            int hasglyph = 0;

            if (offsetEnd <= offset) {
                ctx->compNum++; /* no contours, skip */
            } else if (!sfnt_seek(ttf->glyf, offset, ttf->file)) {
                fputs("Error: failed to find glyph\n", stderr);
                ok = 0;
            } else if (!(hasglyph = sfnt_read_glyf(ttf->file, &glyph))) {
                fputs("Error: failed to read glyph\n", stderr);
                ok = 0;
            } else if (glyph.numContours < 0) {
                if (push_composite_context(&stack, &numStack, glyph.data.composite.components, glyph.data.composite.components, glyph.data.composite.numComponents)) {
                    glyph.data.composite.components = NULL;
                    glyph.data.composite.numComponents = 0;
                } else {
                    fputs("Error: failed to allocate memory\n", stderr);
                    ok = 0;
                }
            } else {
                if (push_composite_context(&stack, &numStack, NULL, NULL, 0)) {
                    ok = simple_glyph_to_float(&glyph, &stack[numStack - 1U].glyf);
                } else {
                    fputs("Error: failed to allocate memory\n", stderr);
                    ok = 0;
                }
            }
            if (hasglyph) sfnt_free_glyf(&glyph);
        }
    }
    while (numStack) {
        free_glyph_float(&stack[--numStack].glyf);
        free(&stack[numStack].mycomps);
    }
    free(stack);
    return ok;
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
        glyph.xMin = glyph.xMax = glyph.yMin = glyph.yMax = 0;
        hasoutline = 1;
    } else if (!sfnt_seek(ttf->glyf, offset, ttf->file)) {
        fputs("Error: failed to find glyph\n", stderr);
    } else if (!(hasglyph = sfnt_read_glyf(ttf->file, &glyph))) {
        fputs("Error: failed to read glyph\n", stderr);
    } else if (glyph.numContours < 0) {
        hasoutline = ttf_composite_outline(ttf, &glyph, c);
    } else if (!(hasoutline = ttf_glyph_outline(&glyph, &c->outline))) {
        fputs("Error: failed to compute outline\n", stderr);
    }
    if (hasoutline) {
        c->xMin = glyph.xMin;
        c->xMax = glyph.xMax;
        c->yMin = glyph.yMin;
        c->yMax = glyph.yMax;
        c->advance = hmetric.advanceWidth;
        c->lsb = hmetric.leftSideBearing;
        if (hasglyph) sfnt_free_glyf(&glyph);
        return 1;
    }
    if (hasglyph) sfnt_free_glyf(&glyph);
    return 0;
}

int ttf_load_chars(const struct TTF* ttf, const char* str, struct Character** c, size_t* numChars, struct CharacterCache* cache) {
    struct Character* dest;
    struct Character* res;
    unsigned int l;
    int ok = 0;

    l = strlen(str);
    if (!(dest = res = malloc(l * sizeof(struct Character)))) {
        fprintf(stderr, "Error: can't allocate memory for Characters\n");
    } else {
        unsigned long cp;
        unsigned int n;
        unsigned char ch;

        ok = 1;
        while (ok && (cp = (unsigned char)*str++)) {
            ok = 0;
            for (n = 0; n < 4U && (cp & (0x80U >> n)); n++);
            if (n >= 4U) break;
            cp &= (0x3FU >> n) | (!n << 6U);
            while (n-- > 1U && (ch = *str++)) cp = (cp << 6U) | (ch & 0x3FU);
            if (cache && character_cache_get(cache, cp, dest)) {
                ok = 1;
            } else if (ttf_load_char(ttf, cp, dest)) {
                if (!(ok = !cache || character_cache_insert(cache, cp, dest))) {
                    character_free(dest);
                }
            } else {
                break;
            }
            dest++;
        }
        if (ok) {
            *c = res;
            *numChars = dest - res;
        } else {
            if (!cache) while (dest > res) character_free(--dest);
            free(res);
        }
    }
    return ok;
}
