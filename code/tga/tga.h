#ifndef _TGA_H_
#define _TGA_H_
#include "../common.h"
#include <stdint.h>


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

typedef struct
{
    SHORT size;
    char authorName[41];
    char authorComment[324];
    SHORT stampMonth;
    SHORT stampDay;
    SHORT stampYear;
    SHORT stampHour;
    SHORT stampMinute;
    SHORT stampSecond;
    char jobName[41];
    SHORT jobHour;
    SHORT jobMinute;
    SHORT jobSecond;
    char softwareId[41];
    SHORT versionNumber;
    char versionLetter;
    LONG keyColor;
    SHORT pixelRatioNumerator;
    SHORT pixelRatioDenominator;
    SHORT gammaNumerator;
    SHORT gammaDenominator;
    LONG colorOffset;
    LONG stampOffset;
    LONG scanOffset;
    BYTE attributesType;
} TGAExtensionArea;

enum ColorMapType{
    NO_COLOR_MAP = 0,
    COLOR_MAPPED = 1
};

enum ImageType{
    NO_IMAGE_DATA = 0,
    UNCOMPRESSED_COLOR_MAPPED = 1,
    UNCOMPRESSED_TRUE_COLOR = 2,
    UNCOMPRESSED_BLACK_AND_WHITE = 3,
    RUN_LENGTH_ENCODED_COLOR_MAPPED = 9,
    RUN_LENGTH_ENCODED_TRUE_COLOR = 10,
    RUN_LENGTH_ENCODED_BLACK_AND_WHITE = 11
};

/*Typedef struct
{
    TGAHeader header;
    char * imageID;

    TGAExtensionArea extensionArea;
} TGAFile;*/

void loadTGA(char * file);

#endif /* _TGA_H_ */


