#ifndef PNG_H
#define PNG_H

int png_read(const char* filename, unsigned int alignRows, unsigned int* width, unsigned int* height, int* alpha, int vReverse, void* buffer);
int png_write(const char* filename, unsigned int alignRows, unsigned int width, unsigned int height, int alpha, int vReverse, const void* buffer);
int png_write_grayscale(const char* filename, unsigned int width, unsigned int height, const void* buffer);

#endif
