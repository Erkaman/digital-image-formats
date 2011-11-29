#ifndef _PNG_H_
#define _PNG_H_

#include "../common.h"
#include "data_list.h"
#include "fixed_data_list.h"

#include "png_defs.h"

typedef struct {
    DataList list;
    size_t position;
} DataStream;

typedef struct {
    INT32 length;
    char chunkType[5];
    FixedDataList chunkData;
    INT32 CRC;
} Chunk;

typedef struct {
    INT32 width;
    INT32 height;

    BYTE bitDepth;
    BYTE colorType;

    BYTE compressionMethod;
    BYTE filterMethod;
    BYTE interlaceMethod;

} ImageHeader;

typedef struct {
    INT32 whitePointX;
    INT32 whitePointY;

    INT32 RedX;
    INT32 RedY;

    INT32 GreenX;
    INT32 GreenY;

    INT32 BlueX;
    INT32 BlueY;

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
    INT32 x;
    INT32 y;

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
    BYTE signature[SIGNATURE_LENGTH];

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
    INT32 * imageGamma;

    SignificantBits * significantBits;

    BYTE * renderingIntent;

    Color * backgroundColor;

    uint16_t * imageHistogram;

    PixelDimensions * pixelDimensions;

    PaletteList * suggestedPalettes;

    TimeStamp * timeStamp;

} PNG_Image;

void dumpPNG(FILE * in, FILE * out);

PNG_Image loadPNG(FILE * in);
void loadSignature(BYTE * signature, FILE * in);

ImageHeader loadImageHeader(FILE * in);
Chunk loadChunk(FILE * in);

void writePNG(PNG_Image image, FILE * out);

void writeSignature(BYTE * signature, FILE * out);

unsigned int crc32(FixedDataList data);


void validateCRC(Chunk chunk);

int isCriticalChunk(Chunk chunk);

#endif /* _PNG_H_ */

