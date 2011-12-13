#ifndef _GIF_H_
#define _GIF_H_

#include <stdint.h>
#include "../common.h"

#define DEBUG 1

/* The GIF standard refers to an unsigned 16-bit number as an
 * "unsigned" for some reason. */
#define UNSIGNED uint16_t

#define FLAG int

/* the color depth of the colors in a GIF is always 24 */
#define COLOR_DEPTH 24
#define TRAILER 0x3b
#define EXTENSION_INTRODUCER 0x21
#define BLOCK_TERMINATOR 0x00

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

    char applicationAuthenticationCode[4];

    GIFDataSubBlocks applicationData;

    BYTE blockTerminator;

} GIFApplicationExtension;

typedef struct{
    BYTE extensionIntroducer;
    BYTE commentLabel;

    char * commentData;

    BYTE blockTerminator;

} GIFCommentExtension;

typedef struct{

    BYTE extensionIntroducer;
    BYTE plainTextLabel;

    BYTE blockSize;

    UNSIGNED textGridLeftPosition;
    UNSIGNED textGridTopPosition;

    UNSIGNED textGridWidth;
    UNSIGNED textGridHeight;

    BYTE characterCellWidth;
    BYTE characterCellHeight;

    BYTE textForegroundColorIndex;
    BYTE textBackgroundColorIndex;

    char * plainTextData;

    BYTE blockTerminator;

} GIFPlainTextExtension;


typedef struct {
    long stringCode;
    long characterCode;

} tableEntry;

typedef struct {
    BYTE r;
    BYTE g;
    BYTE b;
} GIFColor;

void printHelp(void);

void loadGIF(FILE * in, FILE * out);

void loadImageData(FILE * in,FILE * out);

void loadExtension(FILE * in,FILE * out);

int printString(void);

GIFHeader loadHeader(FILE * in);
void printHeader(GIFHeader header,FILE * out);

GIFLogicalScreenDescriptor loadLogicalScreenDescriptor(FILE * in);
void printLogicalScreenDescriptor(
    GIFLogicalScreenDescriptor logicalScreenDescriptor,
    FILE * out);

GIFColor * loadColorTable(int colorTableSize,FILE * in);
void printColorTable(GIFColor * colorTable,int colorTableSize,FILE * out);

void loadImageDescriptor(FILE * in);
void printImageDescriptor(FILE * out);

GIFGraphicControl loadGraphicControl(FILE * in);
void printGraphicControl(GIFGraphicControl graphicControl,FILE * out);

void loadImageColorData(FILE * in);
void printImageColorData(FILE * out);

GIFCommentExtension loadCommentExtension(FILE * in);
void printCommentExtension(GIFCommentExtension comment,FILE * out);

GIFPlainTextExtension loadPlainTextExtension(FILE * in);
void printPlainTextExtension(GIFPlainTextExtension plainText,FILE * out);


void printTableColor(int index,GIFColor * colorTable,FILE * out);
void printColor(int index,GIFColor * colorTable,FILE * out);

GIFApplicationExtension loadApplicationExtension(FILE * in);
void printApplicationExtension(
    GIFApplicationExtension applicationExtension,
    FILE * out);

unsigned int resetCompressionTable(void);

unsigned int inputCode(int codeSize);

void translateCode(unsigned int newCode);

GIFDataSubBlocks readDataSubBlocks(FILE * in);

void printDataSubBlocks(FILE * out,GIFDataSubBlocks subBlocks);
void printBytes(FILE * out,size_t size,BYTE * bytes);

void debugPrint(const char * format, ...);

void printDisposalMethod(GIFGraphicControl graphicControl,FILE * out);

char * subBlocksDataToString(GIFDataSubBlocks subBlocks);

UNSIGNED readUnsigned(FILE * fp);

#endif /* _GIF_H_ */

