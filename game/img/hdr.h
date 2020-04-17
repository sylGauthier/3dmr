#ifndef GAME_IMG_HDR_H
#define GAME_IMG_HDR_H

int hdr_read(const char* filename, unsigned int alignRows, unsigned int* width, unsigned int* height, float** buffer);

unsigned char* hdr_to_ldr(float* hdr, unsigned int width, unsigned height, unsigned int alignRows, int alpha, float gamma, float scale);
float* ldr_to_hdr(unsigned char* ldr, unsigned int width, unsigned height, unsigned int alignRows, int alpha, float gamma, float scale);

#endif
