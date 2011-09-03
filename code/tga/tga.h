#ifndef _TGA_H_
#define _TGA_H_
#include "../common.h"
#include <stdint.h>


/* A short is an unsigned 16-bit integer in the TGA specification. */
#define SHORT uint16_t

/* A short is an unsigned 16-bit integer in the TGA standard. */
#define LONG uint32_t

/* TGA uses ASCII characters in their strings. */
#define CHAR uint8_t

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

typedef struct
{
    SHORT size;
    CHAR authorName[41];
    CHAR authorComment[324];
    SHORT stampMonth;
    SHORT stampDay;
    SHORT stampYear;
    SHORT stampHour;
    SHORT StampMinute;
    SHORT stampSecond;
    CHAR jobName[41];
    SHORT jobHour;
    SHORT jobMinute;
    SHORT jobSecond;
    CHAR softwareId[41];
    SHORT versionNumber;
    CHAR versionLetter;
    LONG keyColor;
    SHORT pixelRatoNumerator;
    SHORT pixelRatioDenominator;
    SHORT gammaNumerator;
    SHORT gammaDenominator;
    LONG colorOffset;
    LONG stampOffset;
    LONG scanOffset;
    BYTE attributesType;
} TGAExtensionArea;

/*typedef struct
{
    TGAHeader header;
    char * imageID;

    TGAExtensionArea extensionArea;
} TGAFile;*/

void loadTGA(char * file);

#endif /* _TGA_H_ */

