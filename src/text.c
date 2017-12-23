#include "text.h"

#include "linear_algebra.h"
#include <stdlib.h>
#include <stdio.h>

struct BitmapGlyph {
	float width;
	float height;
	float texture[4];
};

struct BitmapFont {
	char *name;
	struct BitmapGlyph glyphs[100];
};

struct BitmapFont *create_simple_bitmapFont() {
	struct BitmapFont *font;
	struct BitmapGlyph *glyph;
	int index;
	char c;

	font = malloc(sizeof(struct BitmapFont));
	font->name = "SimpleBitmapFont-9";

	for (c = ' ', index = 0; c <= '~'; c++, index++) {
		glyph = &font->glyphs[index];
		glyph->width = 1;
		glyph->height = 1;

		/* upper left corner */
		glyph->texture[0] = (index % 10) / 10.0;
		glyph->texture[1] = (index / 10) / 10.0;

		/* lower right corner */
		glyph->texture[2] = ((index % 10) + 1) / 10.0;
		glyph->texture[3] = ((index / 10) + 1) / 10.0;
	}

	return font;
}


void append_char(struct BitmapGlyph *glyph, Vec2 origin, float *vertices, float *textures)
{
	/*
     (0,1)   (1,1) | ([0],[1])   ([2],[1])
         +---+     |         O---+
         |  /|     |         |   |
         | / |     |         |   |
         |/  |     |         |   |
         O---+     |         +---+
     (0,0)   (1,0) | ([0],[3])   ([2],[3])
	 */

	/* (0,0) */
	vertices[0] = origin[0];
	vertices[1] = origin[1];
	vertices[2] = 0.0;
	textures[0] = glyph->texture[0];
	textures[1] = glyph->texture[3];

	/* (1,0) */
	vertices[3] = origin[0] + glyph->width;
	vertices[4] = origin[1];
	vertices[5] = 0.0;
	textures[2] = glyph->texture[2];
	textures[3] = glyph->texture[3];

	/* (1,1) */
	vertices[6] = origin[0] + glyph->width;
	vertices[7] = origin[1] + glyph->height;
	vertices[8] = 0.0;
	textures[4] = glyph->texture[2];
	textures[5] = glyph->texture[1];

	/* (0,0) */
	vertices[9] = origin[0];
	vertices[10] = origin[1];
	vertices[11] = 0.0;
	textures[6] = glyph->texture[0];
	textures[7] = glyph->texture[3];

	/* (1,1) */
	vertices[12] = origin[0] + glyph->width;
	vertices[13] = origin[1] + glyph->height;
	vertices[14] = 0.0;
	textures[8] = glyph->texture[2];
	textures[9] = glyph->texture[1];

	/* (0,1) */
	vertices[15] = origin[0];
	vertices[16] = origin[1] + glyph->height;
	vertices[17] = 0.0;
	textures[10] = glyph->texture[0];
	textures[11] = glyph->texture[1];

	origin[0] += glyph->width;
}


int new_text(char *text, int length, struct Mesh* mesh)
{
	static struct BitmapFont *font = 0;
	float *vertices;
	float *textures;
	Vec2 origin = {0};
	char c;
	int i;

	if (!font) {
		font = create_simple_bitmapFont();
	}

	if (!mesh || !text)
		return 1;

	/* use two triangles per character */
	vertices = malloc(2 * 3 * length * 3 * sizeof(float));

	textures = malloc(2 * 3 * length * 2 * sizeof(float));

	for (i = 0; i < length; i++) {
		c = text[i];
		if (' ' <= c && c <= '~') {
			append_char(&font->glyphs[c-' '], origin, vertices+i*18, textures+i*12);
		} else {
			printf("FIXME: current bitmap font doesn't support char: %2x\n", c);
		}
	}

	mesh->vertices = vertices;
	mesh->texCoords = textures;
	mesh->numVertices = length * 2 * 3;
	mesh->numTexCoords = length * 2 * 3;
	mesh->numNormals = 0;
	mesh->numIndices = 0;

	return 0;
}
