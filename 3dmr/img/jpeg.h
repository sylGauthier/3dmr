#ifndef TDMR_IMG_JPEG_H
#define TDMR_IMG_JPEG_H

int jpeg_read_file(const char*      filename,
                   unsigned int     alignRows,
                   unsigned int*    width,
                   unsigned int*    height,
                   unsigned int*    channels,
                   unsigned int     reqChannels,
                   int              vReverse,
                   unsigned char**  buffer);

int jpeg_read_buf(const void*       buf,
                  unsigned int      size,
                  unsigned int      alignRows,
                  unsigned int*     width,
                  unsigned int*     height,
                  unsigned int*     channels,
                  unsigned int      reqChannels,
                  int               vReverse,
                  unsigned char**   buffer);

#endif
