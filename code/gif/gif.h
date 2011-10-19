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

#define DEBUG 1

#define GIFColor uint32_t

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

typedef struct {
    unsigned int stringCode;
    unsigned int characterCode;
} tableEntry;

tableEntry * compressionTable;


/* Used by all images that do not have a local color table */
uint32_t * globalColorTable;

/* used by the graphics that specifies their own local color tables */
int * localColorTable;

/* use a flag to specify whether the current image has local color table or not,
   else use the global color table */

unsigned int nextCode;

char stringCodeStack[40000];
int stackp;

unsigned int ClearCode;
unsigned int EndCode;


GIFHeader header;
GIFLogicalScreenDescriptor logicalScreenDescriptor;
GIFImageDescriptor imageDescriptor;

GIFGraphicControl graphicControl;

void debugPrint(const char * format, ...);

void printHelp(void);
void loadGIF(char * file);

UNSIGNED readUnsigned(FILE * fp);
void readImageInfo(FILE * in);

void loadImageData(FILE * in,FILE * out);
void loadExtension(FILE * in,FILE * out);

char printString(FILE * out);

void printImageInfo(FILE * out);
void printSignature(FILE * out);
void printLogicalScreenDescriptor(FILE * out);
void printGlobalColorTable(FILE * out);
void printTableColor(int index,GIFColor * colorTable,FILE * out);

void printColor(int index,GIFColor * colorTable,FILE * out);

void loadHeader(FILE * in);
void loadLogicalScreenDescriptor(FILE * in);
void loadGlobalColorTable(FILE * in);

void loadImageDescriptor(FILE * in);
void printImageDescriptor(FILE * out);

void loadGraphicControl(FILE * in);
void printGraphicControl(FILE * out);

void loadImageColorData(FILE * in,FILE * out);

void resetCompressionTable(void);

unsigned int inputCode(int codeSize,FILE *input);

void translateCode(unsigned int newCode);


#endif /* _GIF_H_ */
