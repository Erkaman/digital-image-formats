#ifndef _TGA_H_
#define _TGA_H_
#include <stdint.h>

/* A byte is an unsigned 8-bit integer in the TGA specification. */
#define BYTE uint8_t

/* A short is an unsigned 16-bit integer in the TGA specification. */
#define SHORT uint16_t

/* A short is an unsigned 16-bit integer in the TGA standard. */
#define LONG uint32_t

typedef struct
{
    BYTE IDLength;
    BYTE colorMapType;
    BYTE imageType;
    SHORT colorMapStart;
    SHORT colorMapLength;
    BYTE colorMapDepth;
    SHORT xOrigin;
    SHORT yOrigin;
    SHORT width;
    SHORT height;
    BYTE pixelDepth;
    BYTE imageDescriptor;
} TGAHeader;

void loadTGA(char * file);

/* strip the file extension of a filename. */
void strip_extension(char * fileName);

/* getbits:  get n bits from position p */
unsigned getbits(unsigned x, int p, int n);

SHORT readShort(FILE * fp);
BYTE readByte(FILE * fp);


#endif /* _TGA_H_ */

