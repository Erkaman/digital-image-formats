#ifndef _TGA_H_
#define _TGA_H_

#include "../io.h"
#include "../bitwise.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* A short is an unsigned 16-bit integer in the TGA specification. */
#define SHORT uint16_t

/* A short is an unsigned 16-bit integer in the TGA standard. */
#define LONG uint32_t

SHORT readShort(FILE * fp);
LONG readLong(FILE * fp);

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
} TGA_Header;

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
} TGA_ExtensionArea;

typedef struct{

    unsigned long * data;

    SHORT width;
    SHORT height;

} TGA_ImageData;

typedef struct{
    TGA_Header header;

    char imageID[255];

    unsigned long * colorMap;

    TGA_ImageData colorData;

    TGA_ExtensionArea * extensionArea;

    TGA_ImageData postageStamp;

} TGA_Image;

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

void freeTGA_Image(TGA_Image image);
TGA_Image newTGA_Image(void);

TGA_Image loadTGA_Image(FILE * in);
void writeTGA_Image(TGA_Image image, FILE * out);

TGA_Header loadTGA_Header(FILE * in);
void writeTGA_Header(TGA_Header header, FILE * out);

unsigned long * loadColorMap(TGA_Header header, FILE * in);

void writeColorMap(TGA_Header header, unsigned long * colorMap, FILE * out);

void writeTGA_ExtensionArea(TGA_ExtensionArea * extension, FILE * out);

TGA_ExtensionArea * loadTGAExtensionArea(FILE * in);

void loadTGA(char * file);

void writeColorData(unsigned long data,TGA_Header header,FILE * out);

void printFormatAuthorComment(char * authorComment,FILE * fp);
void printRGB(unsigned long r,unsigned long g,unsigned long b,FILE * in);

void printGrayScaleRGB(unsigned long d,FILE * fp);
void printRGBA(unsigned long r,unsigned long g,unsigned long b,unsigned long a,FILE * fp);

TGA_ImageData loadTGA_ImageData(SHORT width, SHORT height,TGA_Header header,unsigned long * colorMap, int compressed, FILE * in);

void writeTGA_ImageData(TGA_ImageData image, TGA_Header header,FILE * out);

void printImageInfo(FILE * out);

TGA_ImageData loadPostageStamp(TGA_Header header,unsigned long * colorMap, LONG offset,FILE * in);

void printHelp(void);

#endif /* _TGA_H_ */
