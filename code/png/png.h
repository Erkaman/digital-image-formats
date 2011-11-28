#ifndef _PNG_H_
#define _PNG_H_

#include "../common.h"
#include <stdint.h>

typedef struct {
    DataContainer signatureData;
} Signature;

typedef struct {
    uint32_t length;
    char chunkType[5];
    DataContainer chunkData;
    uint32_t CRC;
} Chunk;

typedef struct {
    uint32_t width;
    uint32_t height;

    BYTE bitDepth;
    BYTE colorType;

    BYTE compressionMethod;
    BYTE filterMethod;
    BYTE interlaceMethod;

} ImageHeader;

typedef struct {
    uint32_t whitePointX;
    uint32_t whitePointY;

    uint32_t RedX;
    uint32_t RedY;

    uint32_t GreenX;
    uint32_t GreenY;

    uint32_t BlueX;
    uint32_t BlueY;

} PrimaryChromaticities;

typedef struct{
    char profileName[80];

    /* I'm not really sure how the data is stored here.

     See http://www.w3.org/TR/PNG/#11iCCP
     and
    http://www.color.org/ICC1V42.pdf*/
    DataContainer profile;
} ICC_Profile;

/* How this structures should be interpreted depends on the color type used. */
typedef struct {
    BYTE significantGreyscaleBits;

    BYTE significantRedBits;
    BYTE significantGreenBits;
    BYTE significantBlueBits;

    BYTE significantAlphaBits;

} SignificantBits;

typedef struct{
    char keyword[80];

    char * str;
} TextualData;

typedef struct{
    char keyword[80];

    BYTE compressionMethod;

    char * str;
} CompressedTextualData;

typedef struct {
    char keyword[80];

    /* Indicates whether the data is compressed. */
    BYTE compressionFlag;
    /* Indicates the compression method */
    BYTE compressionMethod;

    char * languageTag;

    char * translatedKeyword;

    /* Only the text field may be compressed */
    char * text;

} InternationalTextualData;

typedef struct {

    TextualData * pairsList;

    InternationalTextualData * internationalText;

} textDataList;

typedef struct {
    unsigned long R;
    unsigned long G;
    unsigned long B;
} RGB;

typedef struct {
    unsigned long R;
    unsigned long G;
    unsigned long B;
    unsigned long A;
} RGBA;

typedef union {

    RGB rgb;

    RGBA rgba;

    unsigned long index;

    unsigned long greyscale;

} Color;

typedef struct {

    Color * colors;

    unsigned long size;

} ColorData;


typedef struct {
    ColorData * palette;

    unsigned long size;
} Palette;

typedef struct {
    Palette * pallettes;

    unsigned long size;
} PaletteList;


typedef struct {
    uint32_t x;
    uint32_t y;

    BYTE unitSpecifier;

} PixelDimensions;

typedef struct {
    uint16_t year;

    BYTE month;
    BYTE day;
    BYTE hour;
    BYTE minute;
    BYTE second;
} TimeStamp;

typedef struct {
    Signature signature;

    ImageHeader header;

    /* Put in the proper order. */
    /* The compressed datastream is then the concatenation of the
     * contents of the data fields of all the IDAT chunks. */
    ColorData colorData;

    Palette palette;

    /* Suggested palettes */

    /* ICC Profile. */
    ICC_Profile * profile;

    PrimaryChromaticities * chromaticities;

    /* The gamma chunk contains this and only this value. */
    uint32_t * imageGamma;

    SignificantBits * significantBits;

    BYTE * renderingIntent;

    Color * backgroundColor;

    uint16_t * imageHistogram;

    PixelDimensions * pixelDimensions;

    PaletteList * suggestedPalettes;

    TimeStamp * timeStamp;

} PNG_Image;

/* Header chunk. */
#define IHDR "IHDR"

/* Palette */
#define PLTE "PLTE"

/* Image data chunk. */
#define IDAT "IDAT"

/* Ending chunk. */
#define IEND "IEND"

/* Ending chunk. */
#define tRNS  "tRNS"

#define cHRM  "cHRM"

#define gAMA "gAMA"

#define iCCP "iCCP"

#define sBIT "sBIT"

#define sRGB  "sRGB"

#define tEXt "tEXt"

#define zTXt "zTXt"

#define iTXt "iTXt"

#define bKGD "bKGD"

#define hIST "hIST"

#define pHYs "pHYs"

#define sPLT "sPLT "

#define tIME "tIME"


#define GREYSCALE_COLOR 0
#define TRUECOLOR_COLOR 2
#define INDEXED_COLOR 3
#define GREYSCALE_ALPHA_COLOR 4
#define TRUECOLOR_ALPHA_COLOR 6

/* The only allowed compression method. */
#define DEFLATE_COMPRESSION_METHOD 0

/* The only allowed filter method. */
#define FILTER_METHOD_0 0

/* Filter types: */

#define NO_FILTER 0
#define SUB_FILTER 1
#define UP_FILTER 2
#define AVERAGE_FILTER 3
#define PAETH_FILTER 4

/* Interlacing types. */

#define NO_INTERLACE 0
#define ADAM7_INTERLACE 1

#define PERCEPTUAL_RENDERING_INTENT 0
#define RELATIVE_COLORIMETRIC_RENDERING_INTENT 1
#define SATURATION_RENDERING_INTENT 2
#define ABSOLUTE_COLORIMETRIC_RENDERING_INTENT 3

/* Units specifiers. Used in the pHYs chunk. */

#define UNKNOWN_UNIT 0
#define METRE_UNIT 1

void dumpPNG(FILE * in, FILE * out);

PNG_Image loadPNG(FILE * in);
Signature loadSignature(FILE * in);
ImageHeader loadImageHeader(FILE * in);
Chunk loadChunk(FILE * in);

void writePNG(PNG_Image image, FILE * out);

void writeSignature(Signature signature, FILE * out);

unsigned int crc32(DataContainer data);

void validateCRC(Chunk chunk);

int isCriticalChunk(Chunk chunk);

#endif /* _PNG_H_ */

