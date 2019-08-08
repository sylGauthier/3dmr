#ifndef PNG_H
#define PNG_H

/* channels=[1,4]:
 * 1: grayscale
 * 2: grayscale + alpha
 * 3: RGB
 * 4: RGB + alpha
 * Use one of these in reqChannels to request conversion to a given format, or 0 to specify that no conversion is required (*channels will hold the actual format)
 */

int png_read(const char* filename, unsigned int alignRows, unsigned int* width, unsigned int* height, unsigned int* channels, unsigned int reqChannels, int vReverse, unsigned char** buffer);
int png_write(const char* filename, unsigned int alignRows, unsigned int width, unsigned int height, unsigned int channels, int vReverse, const unsigned char* buffer);

#endif
