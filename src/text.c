#include "text.h"

#include "linear_algebra.h"
#include <stdlib.h>
#include <stdio.h>
#include <GL/glew.h>

#define NB_GLYPHS_PER_ROW 32
#define NB_GLYPHS_PER_LINE NB_GLYPHS_PER_ROW
#define MAX_NB_GLYPHS NB_GLYPHS_PER_ROW * NB_GLYPHS_PER_LINE

struct BitmapFont *simple_bitmap_font(const int size)
{
    struct BitmapFont *font;
    struct BitmapGlyph *glyph;
    int index;
    char c;

    font = malloc(sizeof(struct BitmapFont));
    font->name = "SimpleBitmapFont";
    font->path = "";

    font->nb_glyphs = ('~'-' '+1);
    font->glyphs = malloc(font->nb_glyphs * sizeof(struct BitmapGlyph));

    font->atlas_width = 10;
    font->atlas_height = 10;
    font->texture_atlas = 0;
    font->face = 0;

    for (c = ' ', index = 0; c <= '~'; c++, index++) {
        glyph = &font->glyphs[index];
        glyph->char_code = c;
        glyph->advance_x = size;
        glyph->advance_y = 0;
        glyph->bitmap_top = size;
        glyph->bitmap_left = 0;
        glyph->bitmap_width = size;
        glyph->bitmap_height = size;

        /* upper left corner */
        glyph->texture[0] = (index % 10) / 10.0;
        glyph->texture[1] = (index / 10) / 10.0;

        /* lower right corner */
        glyph->texture[2] = ((index % 10) + 1) / 10.0;
        glyph->texture[3] = ((index / 10) + 1) / 10.0;
    }

    return font;
}

int imgcpy(uint8_t *image, const size_t width, const size_t height,
       const size_t origin_x, const size_t origin_y,
       const FT_Bitmap *glyph)
{
    uint8_t *src = glyph->buffer;
    int offset_image, offset_glyph;
    int i;

    if (!image) {
        return 1;
    }

    if (origin_x + glyph->width >= width
     || origin_y + glyph->rows >= height) {
        /* illegal, glyph outside of buffer bounds */
        printf("ILLEGAL\n");
        return 1;
    }

    for (i = 0; i < glyph->rows; i++) {
        offset_image = (origin_y + i) * width + origin_x;
        offset_glyph = i * glyph->width;
        memcpy(image + offset_image, src + offset_glyph, glyph->width);
    }
    return 0;
}


static FT_Library ft = 0;

struct BitmapFont *ttf_bitmap_font(const char *path, const int font_height)
{
    FT_Face face;
    struct BitmapFont *font;

/* TODO: add cleanup before exit */
    if (!path)
        return NULL;

    if (ft == 0 && FT_Init_FreeType(&ft)) {
        fprintf(stderr, "Could not init freetype library\n");
        return NULL;
    }

    font = malloc(sizeof(struct BitmapFont));
    font->path = path;
    font->size = font_height;

    if (FT_New_Face(ft, path, 0, &font->face)) {
        fprintf(stderr, "Could not open font '%s'\n", path);
        return NULL;
    }
    face = font->face;
    /* auto adjust width based on the given height */
    FT_Set_Pixel_Sizes(face, 0, font_height);
    font->glyphs = calloc(MAX_NB_GLYPHS, sizeof(struct BitmapGlyph));
    font->nb_glyphs = 0;
    font->size = font_height;
    font->atlas_width = NB_GLYPHS_PER_LINE;
    font->atlas_height = NB_GLYPHS_PER_ROW;

    glGenTextures(1, &font->texture_atlas);
    if (font->texture_atlas) {
        glBindTexture(GL_TEXTURE_2D, font->texture_atlas);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED,
                 font_height * font->atlas_width,
                 font_height * font->atlas_height,
                 0, GL_RED, GL_UNSIGNED_BYTE, 0);
    } else {
        printf("ERROR no texture\n");
        return NULL;
    }

    return font;
}

void append_glyph(const struct BitmapGlyph *glyph, Vec2 origin, float *vertices, float *textures)
{
    const float left   = origin[0] + glyph->bitmap_left;
    const float top    = origin[1] + glyph->bitmap_top;
    const float right  = left + glyph->bitmap_width;
    const float bottom = top  - glyph->bitmap_height;

    /*
     *      VERTICES    |      TEXTURE_COOR
     *  ----------------+------------------------
     *    (0,1)   (1,1) | ([0],[1])   ([2],[1])
     *        +---+     |         O---+
     *        |  /|     |         |   |
     *        | / |     |         |   |
     *        |/  |     |         |   |
     *        O---+     |         +---+
     *    (0,0)   (1,0) | ([0],[3])   ([2],[3])
     */

    /* (0,0) */
    vertices[0] = left;
    vertices[1] = bottom;
    vertices[2] = 0.0;
    textures[0] = glyph->texture[0];
    textures[1] = glyph->texture[3];

    /* (1,0) */
    vertices[3] = right;
    vertices[4] = bottom;
    vertices[5] = 0.0;
    textures[2] = glyph->texture[2];
    textures[3] = glyph->texture[3];

    /* (1,1) */
    vertices[6] = right;
    vertices[7] = top;
    vertices[8] = 0.0;
    textures[4] = glyph->texture[2];
    textures[5] = glyph->texture[1];

    /* (0,0) */
    vertices[9]  = left;
    vertices[10] = bottom;
    vertices[11] = 0.0;
    textures[6]  = glyph->texture[0];
    textures[7]  = glyph->texture[3];

    /* (1,1) */
    vertices[12] = right;
    vertices[13] = top;
    vertices[14] = 0.0;
    textures[8] = glyph->texture[2];
    textures[9] = glyph->texture[1];

    /* (0,1) */
    vertices[15] = left;
    vertices[16] = top;
    vertices[17] = 0.0;
    textures[10] = glyph->texture[0];
    textures[11] = glyph->texture[1];

    origin[0] += glyph->advance_x;
    origin[1] += glyph->advance_y;
}


struct BitmapGlyph *
find_glyph(struct BitmapFont *font, uint32_t char_code)
{
    int i;

    for (i = 0; i < font->nb_glyphs; i++) {
        if (font->glyphs[i].char_code == char_code) {
            return &font->glyphs[i];
        }
    }

    return NULL;
}

struct BitmapGlyph *
add_glyph(struct BitmapFont *font, uint32_t char_code)
{
    struct BitmapGlyph *glyph;
    FT_Face face;
    int index, x, y;

    face = font->face;

    if (FT_Load_Char(face, char_code, FT_LOAD_RENDER)) {
        fprintf(stderr, "Could not load glyph U+%4x \n", char_code);
        return NULL;
    }

    index = font->nb_glyphs;
    glyph = &font->glyphs[index];
    font->nb_glyphs++;

    glyph->char_code = char_code;

    x = index % font->atlas_width;
    y = index / font->atlas_width;

    glyph->advance_x = face->glyph->advance.x >> 6; /* / 64 */
    glyph->advance_y = face->glyph->advance.y >> 6; /* / 64 */

    glyph->bitmap_left = face->glyph->bitmap_left;
    glyph->bitmap_top= face->glyph->bitmap_top;

    glyph->bitmap_width = face->glyph->bitmap.width;
    glyph->bitmap_height = face->glyph->bitmap.rows;

    glBindTexture(GL_TEXTURE_2D, font->texture_atlas);
    glTexSubImage2D(GL_TEXTURE_2D, 0,
            x * font->size,    y * font->size,
            face->glyph->bitmap.width, face->glyph->bitmap.rows,
            GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);

    /* top left corner */
    glyph->texture[0] = x / (float)font->atlas_width;
    glyph->texture[1] = y / (float)font->atlas_height;

    /* low right corner */
    glyph->texture[2] = glyph->texture[0]
        + glyph->bitmap_width / (float)(font->atlas_width * font->size);
    glyph->texture[3] = glyph->texture[1]
        + glyph->bitmap_height / (float)(font->atlas_height * font->size);

    return glyph;
}

const struct BitmapGlyph *
load_glyph(struct BitmapFont *font, uint32_t char_code)
{
    const struct BitmapGlyph *glyph;

    glyph = find_glyph(font, char_code);

    if (!glyph)
        glyph = add_glyph(font, char_code);

    return glyph;
}

uint32_t utf8_to_utf32(const char *s, int *len) {
    uint32_t out = 0;
    uint8_t c;
    int nb;

    c = s[0];
    if (c >= 0xC0) { /* 0b11000000 */
        if (c >= 0xF0) { /* 0b11110000 */
            out = (c & 0x7);
            nb = 3;
        } else if (c >= 0xE0) { /* 0b11100000 */
            out = (c & 0xF);
            nb = 2;
        } else { /* c >= 0xC0 */
            out = (c & 0x1F);
            nb = 1;
        }
        for (; nb > 0; nb--, (*len)++) {
            s++;
            c = s[0];
            if (c != 0 && c > 127/*(c & 0xC0) == 0x80*/) {
                /* 0b10000000 */
                out = (out << 6) | (c & 0x3F);
            } else {
                break;
            }
        }
    } else {
        out = c;
    }

    return out;
}

int new_text(struct BitmapFont *font, const char *text, struct Mesh *mesh)
{
    const struct BitmapGlyph *glyph;
    float *vertices;
    float *textures;
    Vec2 origin = {-1,0};
    uint32_t char_code;
    int length, i, index;

    if (!font) {
        return 1;
    }

    if (!mesh || !text)
        return 1;

    length = strlen(text);

    /* use two triangles per character */
    vertices = malloc(2 * 3 * length * 3 * sizeof(float));

    textures = malloc(2 * 3 * length * 2 * sizeof(float));

    for (index = i = 0; i < length; i++) {
        char_code = utf8_to_utf32(&text[i], &i);

        glyph = load_glyph(font, char_code);
        if (glyph) {
            append_glyph(glyph, origin, vertices+index*18, textures+index*12);
            index++;
        } else {
            printf("Error: font '%s' do not support glyph TODO\n", font->name);
        }
    }

    mesh->vertices = vertices;
    mesh->normals = NULL;
    mesh->texCoords = textures;
    mesh->indices = NULL;
    mesh->numVertices = index * 2 * 3;
    mesh->hasNormals = 0;
    mesh->hasTexCoords = 1;
    mesh->numIndices = 0;

    return 0;
}

void font_free(struct BitmapFont *font) {
    free(font->glyphs);
    free(font);
}
