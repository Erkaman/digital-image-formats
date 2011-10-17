#ifndef _GIF_H_
#define _GIF_H_

#include <stdint.h>
#include "../common.h"

/* The GIF standard refers to an unsigned 16-bit number as an
 * "unsigned" for some reason. */
#define UNSIGNED uint16_t

#define FLAG int

typedef struct{
    char signature[4];
    char version[4];
} GIFHeader;

typedef struct{
    UNSIGNED logicalScreenWidth;
    UNSIGNED logicalScreenHeight;

    /* packed fields */
    FLAG globalColorTableFlag;
    BYTE colorResolution;
    FLAG sortFlag;
    BYTE globalColorTableSize;

    BYTE backgroundColorIndex;
    BYTE pixelAspectRatio;

} GIFLogicalScreenDescriptor;

#endif /* _GIF_H_ */
