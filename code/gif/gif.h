#ifndef _GIF_H_
#define _GIF_H_

#include <stdint.h>
#include "../common.h"

/* The GIF standard refers to an unsigned 16-bit number as an
 * "unsigned" for some reason. */
#define UNSIGNED uint16_t

#define FLAG int

/* the color depth of the colors in a GIF is always 24 */
#define COLOR_DEPTH 24
#define TRAILER 0x3b
#define EXTENSION_INTRODUCER 0x21

#define GRAPHIC_CONTROL_LABEL 0xf9
#define COMMENT_LABEL 0xfe
#define APPLICATION_EXTENSION_LABEL 0xff

#define PLAIN_TEXT_LABEL 0x01
#define IMAGE_SEPARATOR 0x2c


typedef struct{
    char signature[4];
    char version[4];
} GIFHeader;

typedef struct {
    BYTE * data;
    unsigned long size;
} GIFDataSubBlocks;

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

typedef struct{
    BYTE imageSeparator;

    UNSIGNED imageLeftPosition;
    UNSIGNED imageTopPosition;

    UNSIGNED imageWidth;
    UNSIGNED imageHeight;

    /* packed fields */
    FLAG localColorTableFlag;
    FLAG interlaceFlag;
    FLAG sortFlag;
    BYTE reserved;
    BYTE localColorTableSize;

} GIFImageDescriptor;

typedef struct{
    BYTE extensionIntroducer;
    BYTE graphicControlLabel;
    BYTE blockSize;

    /* packed fields */
    BYTE reserved;
    BYTE disposalMethod;
    FLAG userInputFlag;
    FLAG transparencyFlag;

    UNSIGNED delayTime;
    BYTE transparencyIndex;
    BYTE blockTerminator;

} GIFGraphicControl;

typedef struct{
    BYTE extensionIntroducer;
    BYTE extensionLabel;

    BYTE blockSize;

    char applicationIdentifier[9];

    BYTE applicationAuthenticationCode[3];

/*    BYTE * applicationData;
    unsigned long applicationDataLength;*/

    GIFDataSubBlocks applicationData;

    BYTE blockTerminator;

} GIFApplicationExtension;

typedef struct {
    unsigned int stringCode;
    unsigned int characterCode;

} tableEntry;

typedef struct {
    BYTE r;
    BYTE g;
    BYTE b;
} GIFColor;

void printHelp(void);
void loadGIF(char * file);

void loadImageData(FILE * in,FILE * out);

void loadExtension(FILE * in,FILE * out);

char printString(void);


void printHeader(GIFHeader header,FILE * out);

void printLogicalScreenDescriptor(FILE * out);
void printGlobalColorTable(FILE * out);
void printTableColor(int index,GIFColor * colorTable,FILE * out);

void printColor(int index,GIFColor * colorTable,FILE * out);

GIFHeader loadHeader(FILE * in);

void loadLogicalScreenDescriptor(FILE * in);
void loadGlobalColorTable(FILE * in);

void loadImageDescriptor(FILE * in);
void printImageDescriptor(FILE * out);

GIFGraphicControl loadGraphicControl(FILE * in);
void printGraphicControl(GIFGraphicControl graphicControl,FILE * out);

void loadImageColorData(FILE * in);
void printImageColorData(FILE * out);

unsigned int resetCompressionTable(void);

unsigned int inputCode(int codeSize);

void translateCode(unsigned int newCode);

void newSubBlock(FILE * in);

GIFApplicationExtension loadApplicationExtension(FILE * in);

void printApplicationExtension(
    GIFApplicationExtension applicationExtension,
    FILE * out);

void readBytes(FILE * in,size_t length,BYTE * bytes);

GIFDataSubBlocks readDataSubBlocks(FILE * in);

void printDataSubBlocks(FILE * out,GIFDataSubBlocks subBlocks);
void printBytes(FILE * out,size_t size,BYTE * bytes);

void freeDataSubBlocks(GIFDataSubBlocks dataSubBlocks);


#endif /* _GIF_H_ */

