#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "gif.h"

/* Used by all images that do not have a local color table */
unsigned long * globalColorTable;

/* used by the graphics that specifies their own local color tables */
int * localColorTable;

/* use a flag to specify whether the current image has local color table or not,
   else use the global color table */

GIFHeader header;
GIFLogicalScreenDescriptor logicalScreenDescriptor;
GIFImageDescriptor imageDescriptor;

GIFGraphicControl graphicControl;

void printHelp(void);
void loadGIF(char * file);

UNSIGNED readUnsigned(FILE * fp);
void readImageInfo(FILE * in);

void loadImageData(FILE * in,FILE * out);
void loadExtension(FILE * in,FILE * out);

void printImageInfo(FILE * out);
void printSignature(FILE * out);
void printLogicalScreenDescriptor(FILE * out);
void printGlobalColorTable(FILE * out);
void printTableColor(int index,unsigned long * colorTable,FILE * out);

void loadHeader(FILE * in);
void loadLogicalScreenDescriptor(FILE * in);
void loadGlobalColorTable(FILE * in);

void loadImageDescriptor(FILE * in);
void printImageDescriptor(FILE * out);

void loadGraphicControl(FILE * in);
void printGraphicControl(FILE * out);


/* the color depth of the colors in a GIF is always 24 */
#define COLOR_DEPTH 24

#define TRAILER 0x3b
#define EXTENSION_INTRODUCER 0x21

#define GRAPHIC_CONTROL_LABEL 0xf9
#define COMMENT_LABEL 0xfe
#define APPLICATION_EXTENSION_LABEL 0xff
#define PLAIN_TEXT_LABEL 0x01

#define IMAGE_SEPARATOR 0x2c

int main(int argc, char *argv[])
{
    if(argc == 1){
        printf("No file was specified for loading\n");
        printf("Try --help for more information.\n");
        return 1;
    } else
        if(!strcmp("--help",argv[1]))
            printHelp();
    loadGIF(argv[1]);

    return 0;
}

UNSIGNED readUnsigned(FILE * fp)
{
    UNSIGNED s;
    fread(&s,sizeof(UNSIGNED),1,fp);
    return s;
}

void printHelp(void)
{
    printf("Usage: gif IN\n");
    printf("Dump the information and color data of a GIF file.\n");
    printf("  --help\tDisplay this help message.\n");

}

void loadGIF(char * file)
{
    FILE * in;
    FILE * out;

    in = fopen(file,"rb");
    assertFileOpened(in);

    file = changeExtension(file,"dmp");
    out = fopen(file,"wb");
    assertFileOpened(out);
    free(file);

    /* read the image info; things like color table and headers */
    readImageInfo(in);

    fprintf(out,"* Image info:\n");

    printImageInfo(out);


    fprintf(out,"* Color data:\n");

    loadImageData(in,out);



    fclose(in);

    /* causes segfault for some reason; fix later */
    fclose(out);
}

void readImageInfo(FILE * in)
{
    loadHeader(in);
    loadLogicalScreenDescriptor(in);
    if(logicalScreenDescriptor.globalColorTableFlag)
        loadGlobalColorTable(in);
}

void loadHeader(FILE * in)
{
    readStr(in,3,header.signature);
    header.signature[3] = '\0';

    readStr(in,3,header.version);
    header.version[3] = '\0';
}

void loadLogicalScreenDescriptor(FILE * in)
{
    BYTE packedFields;

    logicalScreenDescriptor.logicalScreenWidth = readUnsigned(in);
    logicalScreenDescriptor.logicalScreenHeight = readUnsigned(in);

    packedFields = readByte(in);
    logicalScreenDescriptor.globalColorTableFlag = (packedFields & (1 << 7)) >> 7;
    logicalScreenDescriptor.colorResolution = (packedFields & (7 << 4)) >> 4;
    logicalScreenDescriptor.sortFlag = (packedFields & (1 << 3)) >> 3;
    logicalScreenDescriptor.globalColorTableSize = (packedFields & 7);

    logicalScreenDescriptor.backgroundColorIndex = readByte(in);
    logicalScreenDescriptor.pixelAspectRatio = readByte(in);
}

void printImageInfo(FILE * out)
{
    printSignature(out);
    printLogicalScreenDescriptor(out);
    if(logicalScreenDescriptor.globalColorTableFlag)
        printGlobalColorTable(out);
}

void printSignature(FILE * out)
{
    fprintf(out,"** Header:\n");
    fprintf(out,"Signature:%s\n",header.signature);
    fprintf(out,"Version:%s\n",header.version);
}

void printLogicalScreenDescriptor(FILE * out)
{
    fprintf(out,"** Logical Screen Descriptor:\n");

    fprintf(out,"Logical Screen Width:%d\n",
            logicalScreenDescriptor.logicalScreenWidth);
    fprintf(out,"Logical Screen Height:%d\n",
            logicalScreenDescriptor.logicalScreenHeight);

    fprintf(out,"Global Color Table Flag:%d\n",
            logicalScreenDescriptor.globalColorTableFlag);
    fprintf(out,"Color Resolution:%d\n",
            logicalScreenDescriptor.colorResolution);
    fprintf(out,"Sort Flag:%d\n",
            logicalScreenDescriptor.sortFlag);
    fprintf(out,"Global Color Table Size:%d\n",
            logicalScreenDescriptor.globalColorTableSize);

    fprintf(out,"Background Color Index:%d\n",
            logicalScreenDescriptor.backgroundColorIndex);
    fprintf(out,"Pixel Aspect Ratio:%d\n",
            logicalScreenDescriptor.pixelAspectRatio);
}

void loadGlobalColorTable(FILE * in)
{

    unsigned long color;
    int realGlobalColorTableSize;
    int i;

    realGlobalColorTableSize = pow(2,1 + logicalScreenDescriptor.globalColorTableSize);

    globalColorTable = (unsigned long *) malloc(realGlobalColorTableSize);

    for(i = 0; i <  realGlobalColorTableSize; ++i){
        color  = 0;
        fread(&color, COLOR_DEPTH / 8, 1, in);

        /* This line for some weird causes a segfault at flose(out). */
        globalColorTable[i] = color;
    }
}

void printGlobalColorTable(FILE * out)
{
    int realGlobalColorTableSize;
    int i;

    fprintf(out,"** Global Color Table:\n");

    realGlobalColorTableSize = pow(2,1 + logicalScreenDescriptor.globalColorTableSize);

    for(i = 0; i <  realGlobalColorTableSize; ++i)
        printTableColor(i,globalColorTable,out);
}

void printTableColor(int index,unsigned long * colorTable,FILE * out)
{
    unsigned long r,g,b;
    unsigned long color;

    color = colorTable[index];

    /* because the color is stored least significant bit first. */
    b = (color & (0xff << 16)) >> 16;
    g = (color & (0xff << 8)) >> 8;
    r = color & 0xff;

    out = out;
    fprintf(out,"%d:(%lu,%lu,%lu)\n",index,r,g,b);
}

void loadImageData(FILE * in,FILE * out)
{
    /* also called separator for image descriptors */
    BYTE introducer;

    introducer = readByte(in);


    while(introducer != TRAILER){

        switch(introducer){
        case EXTENSION_INTRODUCER:
            loadExtension(in,out);
            break;
        case IMAGE_SEPARATOR:

	    loadImageDescriptor(in);
	    printImageDescriptor(out);

	    /* perform the decompression of the image data. */
	    introducer = TRAILER;

            break;
        }

	introducer = readByte(in);
    }
}

void loadExtension(FILE * in,FILE * out)
{
    /* Identifies the extension type. */
    BYTE extensionLabel;

    out = out;

    extensionLabel = readByte(in);

    switch(extensionLabel){
    case GRAPHIC_CONTROL_LABEL:
	loadGraphicControl(in);
	printGraphicControl(out);
	/* load image(same thing as when finding the image separator) */
        break;
    case COMMENT_LABEL:
        break;
    case APPLICATION_EXTENSION_LABEL:
        break;
    case PLAIN_TEXT_LABEL:
        break;
    }
}

void loadImageDescriptor(FILE * in)
{
    int packedFields;

    imageDescriptor.imageSeparator = IMAGE_SEPARATOR;

    imageDescriptor.imageLeftPosition = readUnsigned(in);
    imageDescriptor.imageTopPosition = readUnsigned(in);
    imageDescriptor.imageWidth = readUnsigned(in);
    imageDescriptor.imageHeight = readUnsigned(in);

    packedFields = readByte(in);

    imageDescriptor.localColorTableFlag = (packedFields & (1 << 7)) >> 7;
    imageDescriptor.interlaceFlag = (packedFields & (1 << 6)) >> 6;
    imageDescriptor.sortFlag = (packedFields & (1 << 5)) >> 5;
    imageDescriptor.reserved = (packedFields & (3 << 3)) >> 3;
    imageDescriptor.localColorTableSize = (packedFields & 7);
}

void printImageDescriptor(FILE * out)
{
    fprintf(out,"** Image Descriptor:\n");

    fprintf(out,"Image Separator:%d\n",imageDescriptor.imageSeparator);

    fprintf(out,"Image Left Position:%d\n",imageDescriptor.imageLeftPosition);
    fprintf(out,"Image Top Position:%d\n",imageDescriptor.imageTopPosition);
    fprintf(out,"Image Width:%d\n",imageDescriptor.imageWidth);
    fprintf(out,"Image Height:%d\n",imageDescriptor.imageHeight);

    fprintf(out,"Local Color Table Flag:%d\n",imageDescriptor.localColorTableFlag);
    fprintf(out,"Interlace Flag:%d\n",imageDescriptor.interlaceFlag);
    fprintf(out,"Sort Flag:%d\n",imageDescriptor.sortFlag);
    fprintf(out,"Reversed:%d\n",imageDescriptor.reserved);
    fprintf(out,"Local Color Table Size:%d\n",imageDescriptor.localColorTableSize);
}

void loadGraphicControl(FILE * in)
{
    BYTE packedFields;

    graphicControl.extensionIntroducer = EXTENSION_INTRODUCER;
    graphicControl.graphicControlLabel = GRAPHIC_CONTROL_LABEL;

    graphicControl.blockSize = readByte(in);

    packedFields = readByte(in);

    graphicControl.reserved = (packedFields & (7 << 5) >> 5);
    graphicControl.disposalMethod = (packedFields & (7 << 2) >> 2);
    graphicControl.userInputFlag = ((packedFields & (1 << 1)) >> 1);
    graphicControl.transparencyFlag = (packedFields & 1);

    graphicControl.delayTime = readUnsigned(in);
    graphicControl.transparencyIndex = readByte(in);
    graphicControl.blockTerminator = readByte(in);
}

void printGraphicControl(FILE * out)
{
    fprintf(out,"** Graphic Control\n");

    fprintf(out,"Extension Introducer: %d\n",graphicControl.extensionIntroducer);
    fprintf(out,"Graphics Control Label: %d\n",graphicControl.graphicControlLabel);
    fprintf(out,"Block Size: %d\n",graphicControl.blockSize);

    fprintf(out,"Reserved: %d\n",graphicControl.reserved);
    fprintf(out,"Disposal Method: %d\n",graphicControl.disposalMethod);
    fprintf(out,"User Input Flag: %d\n",graphicControl.userInputFlag);
    fprintf(out,"Transparency Flag: %d\n",graphicControl.transparencyFlag);

    fprintf(out,"Delay Time: %d\n",graphicControl.delayTime);
    fprintf(out,"Transparency Index: %d\n",graphicControl.transparencyIndex);
    fprintf(out,"Block Terminator: %d\n",graphicControl.blockTerminator);
}
