#ifndef _PNG_H_
#define _PNG_H_

#include "../io.h"
#include "../data_stream.h"
#include "../bits.h"

#include "png_defs.h"
#include "../deflate/zlib.h"

typedef struct {
    INT32 length;
    char type[5];
    std::vector<BYTE> data;
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

    int numChannels;
    int channelBitDepth;

} ColorInfo;

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
    std::vector<BYTE> profile;
} ICC_Profile;

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
    INT32 R;
    INT32 G;
    INT32 B;
} RGB;

typedef struct {
    INT32 R;
    INT32 G;
    INT32 B;
    INT32 A;
} RGBA;

typedef struct {
    INT32 greyscale;
    INT32 alpha;
} GreyscaleAlpha;

typedef union {

    RGB rgb;

    RGBA rgba;

    INT32 index;

    INT32 greyscale;

    GreyscaleAlpha greyscaleAlpha;

} Color;

typedef struct {
    INT32 x;
    INT32 y;

    BYTE unitSpecifier;

} PixelDimensions;

typedef struct {

    char name[80];

    /* of SuggestedPaletteEntry */
    std::vector<Color> entry;

} SuggestedPalette;

typedef struct {

    INT16 R;
    INT16 G;
    INT16 B;
    INT16 A;

    INT16 frequency;

} SuggestedPaletteEntry;

typedef struct {
    INT16 year;

    BYTE month;
    BYTE day;
    BYTE hour;
    BYTE minute;
    BYTE second;
} TimeStamp;

typedef struct {

    Color color;

    std::vector<BYTE> transparentIndices;

} Transparency;

typedef struct {
    BYTE signature[SIGNATURE_LENGTH];

    ImageHeader header;

    /* Put in the proper order. */
    /* The compressed datastream is then the concatenation of the
     * contents of the data fields of all the IDAT chunks. */
    std::vector<Color> colorData;

    std::vector<Color> * palette;

    /* Suggested palettes */

    /* ICC Profile. */
    ICC_Profile * profile;

    PrimaryChromaticities * chromaticities;

    /* The gamma chunk contains this and only this value. */
    INT32 * imageGamma;

    SignificantBits * significantBits;

    BYTE * renderingIntent;

    Color * backgroundColor;

    /* INT16 */
    std::vector<INT16> * imageHistogram;

    PixelDimensions * pixelDimensions;

    TimeStamp * timeStamp;

    /* Of type TextualData */
    std::vector<TextualData> textDataList;

    /* Of type InternationalTextualData */
    std::vector<InternationalTextualData> internationalTextDataList;
    Transparency * transparency;

    std::vector<SuggestedPalette> * suggestedPalettes;

} PNG_Image;

typedef struct {
    size_t width;
    size_t height;
} Size;

typedef struct {
    Size sizes[7];
} InterlacedSubImagesSizes;

InterlacedSubImagesSizes calcInterlacedSubImagesSizes(size_t width, size_t height);

InterlacedSubImagesSizes calcImageSize(size_t width, size_t height);

void dumpPNG(FILE * in, FILE * out);

PNG_Image loadPNG(FILE * in);

PNG_Image getEmptyPNG_Image(void);

void loadSignature(BYTE * signature, FILE * in);
std::vector<INT16> * loadImageHistogram(DataStream & stream);

BYTE * loadRenderingIntent(DataStream & stream);
void writeRenderingIntent(BYTE * renderingIntent, FILE * out);

std::vector<Color> * loadPalette(DataStream & stream);

void writePalette(PNG_Image image,FILE * out);

INT32 * loadImageGamma(DataStream stream);
void writeImageGamma(INT32 * imageGamma, FILE * out);

TimeStamp * loadTimeStamp(DataStream stream);
void writeTimeStamp(TimeStamp * timeStamp, FILE * out);

PixelDimensions * loadPixelDimensions(DataStream streamn);
void writePixelDimensions(PixelDimensions * pixelDimensions, FILE * out);

void writeImageHistogram(std::vector<INT16> * imageHistogram, FILE * out);

Color * loadBackgroundColor(ImageHeader header, DataStream stream);
void writeBackgroundColor(
    PNG_Image image,
    Color * backgroundColor,
    FILE * out);

void writeColor(PNG_Image image,
                Color color,
                FILE * out,
                int backgroundColor);


SignificantBits * loadSignificantBits(ImageHeader header, DataStream stream);

void writeSignificantBits(
    SignificantBits * significantBits,
    ImageHeader header,
    FILE * out);

PrimaryChromaticities * loadPrimaryChromaticities(DataStream stream);

void writePrimaryChromaticities(
    PrimaryChromaticities * primaryChromaticities,
    FILE * out);

TextualData loadTextualData(
    DataStream stream,
    int compressed,
    INT32 chunkLength);


void freeTextualData(void * textualData);

void writePNG(PNG_Image image, FILE * out);
void freePNG_Image(PNG_Image image);

Chunk loadChunk(FILE * in);

void writeSignature(BYTE * signature, FILE * out);
void writeHeader(ImageHeader header, FILE * out);

std::vector<Color> loadColorData(std::vector<BYTE> data, ImageHeader header);

Transparency * loadTransparency(const ImageHeader & header, DataStream & stream);

void writeColorData(PNG_Image image, FILE * out);


void writeTextDataList(std::vector<TextualData> textDataList, FILE * out);


unsigned int crc32(std::vector<BYTE> data);


void validateCRC(Chunk chunk);

int isCriticalChunk(Chunk chunk);
int isChunkType(Chunk chunk, const char * chunkType);

INT32 getMaximumChannelValue(ImageHeader header);

std::vector<BYTE> unfilterImage(std::vector<BYTE> data, ImageHeader header);

std::vector<Color> splitUpColorData(std::vector<BYTE> data, ImageHeader header);


std::vector<Color> splitUpColorDataSubImage(
    DataStream & colorStream,
    BitReader & inBits,
    const ImageHeader & header,
    size_t width,
    size_t height,
    size_t dataCount
    );

ColorInfo getColorInfo(ImageHeader header);

BYTE compute_a(size_t i, size_t bpp, const std::vector<BYTE> & unfiltered);

BYTE compute_b(size_t scanline, size_t width, const std::vector<BYTE> & unfiltered);

BYTE compute_c(
    size_t i,
    size_t bpp,
    size_t scanline,
    size_t width,
    const std::vector<BYTE> & unfiltered);

unsigned int paethPredictor(unsigned int a, unsigned int b, unsigned int c);

unsigned long readNextChannel(
    DataStream & stream,
    BitReader & inBits,
    const ImageHeader & header);
std::vector<Color> uninterlace(const std::vector<Color> & data, const ImageHeader & header);

std::vector<BYTE> unfilterSubImage(
    DataStream & stream,
    size_t imageWidth,
    size_t height,
    const ColorInfo & info);

ImageHeader loadImageHeader(FILE * in);

#endif /* _PNG_H_ */
