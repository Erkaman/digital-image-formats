#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "gif.h"
#include "util.h"

BYTE * subBlockBytes;
int subBlockSize;
int subBlockIndex;

tableEntry * compressionTable;

/* Used by all images that do not have a local color table */
GIFColor * globalColorTable;

/* used by the graphics that specifies their own local color tables */
int * localColorTable;

/* use a flag to specify whether the current image has local color table or not,
   else use the global color table */

char stringCodeStack[40000];
int stackp;

unsigned int ClearCode;
unsigned int EndCode;

/* structures */
GIFHeader header;
GIFLogicalScreenDescriptor logicalScreenDescriptor;
GIFImageDescriptor imageDescriptor;
GIFGraphicControl graphicControl;

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

    /* read the image info; things like color table and headers */
    readImageInfo(in);

    debugPrint("Image Info\n");
    fprintf(out,"* Image info:\n");

    printImageInfo(out);

    debugPrint("Color Data\n");
    fprintf(out,"* Color data:\n");

    loadImageData(in,out);

    fclose(in);

    /* causes segfault for some reason; fix later */
    fclose(out);
    free(file);
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
    int realGlobalColorTableSize;
    int i;

    realGlobalColorTableSize = pow(2,1 + logicalScreenDescriptor.globalColorTableSize);

    globalColorTable = (GIFColor *) malloc(sizeof(GIFColor) * realGlobalColorTableSize);

    for(i = 0; i <  realGlobalColorTableSize; ++i){

        /* This line for some weird causes a segfault at flose(out). */
        globalColorTable[i].r = readByte(in);
        globalColorTable[i].g = readByte(in);
        globalColorTable[i].b = readByte(in);
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

void printTableColor(int index,GIFColor * colorTable,FILE * out)
{
    GIFColor color = colorTable[index];

    fprintf(out,"%d:(%d,%d,%d)\n",index,color.r,color.g,color.b);
}

void printColor(int index,GIFColor * colorTable,FILE * out)
{
    static int col = 0;

    GIFColor color;

    color = colorTable[index];

    fprintf(out,"(%d,%d,%d)",color.r,color.g,color.b);

    ++col;

    if(col == logicalScreenDescriptor.logicalScreenWidth){
        fprintf(out,"\n");
        col = 0;
    }
}

void loadImageData(FILE * in,FILE * out)
{
    /* also called separator for image descriptors */
    BYTE introducer;

    introducer = readByte(in);

    while(introducer != TRAILER){

        switch(introducer){
        case EXTENSION_INTRODUCER:
            debugPrint("EXTENSION_INTRODUCER\n");
            loadExtension(in,out);
            break;
        case IMAGE_SEPARATOR:
            debugPrint("IMAGE_SEPARATOR\n");

            /* if local color table size > 0 load local color table */
            loadImageDescriptor(in);
            printImageDescriptor(out);

            debugPrint("Image descriptor loaded\n");

            /* perform the decompression of the image data. */
            loadImageColorData(in,out);
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
        debugPrint("GRAPHIC_CONTROL_LABEL\n");

        loadGraphicControl(in);
        printGraphicControl(out);
        /* load image(same thing as when finding the image separator) */
        break;
    case COMMENT_LABEL:

        debugPrint("COMMENT_LABEL\n");
        break;
    case APPLICATION_EXTENSION_LABEL:
        debugPrint("APPLICATION_EXTENSION_LABEL\n");
        break;
    case PLAIN_TEXT_LABEL:
        debugPrint("PLAIN_TEXT_LABEL\n");
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

void translateCode(unsigned int newCode)
{
    tableEntry entry;

    entry = compressionTable[newCode];

    while(1){

        stringCodeStack[stackp++] = entry.characterCode;

        if(entry.stringCode == ((unsigned int)-1) )
            break;
        else
            entry = compressionTable[entry.stringCode];
    }
}

void loadImageColorData(FILE * in,FILE * out)
{
    int codeSize;

    unsigned int oldCode;
    unsigned int newCode;
    char character;

    unsigned int nextCode;

    /* allocate an array big enough for all sub blocks.  */
    subBlockBytes = (BYTE *) malloc(sizeof(BYTE) * SUB_BLOCKS_MAX_SIZE);

    stackp = 0;

    /* do this at the beginning of the program? */
    compressionTable = (tableEntry *)malloc(pow(2,12));

    codeSize = readByte(in) + 1;
    debugPrint("codeSize:%d\n",codeSize);

    newSubBlock(in);

    /* Skip the clear code. */
    inputCode(codeSize);
    nextCode = resetCompressionTable();

    oldCode = inputCode(codeSize);

    printColor(oldCode,globalColorTable,out);

    character = oldCode;
    newCode = inputCode(codeSize);

    /* TODO: handle clear codes. */
    while(newCode != EndCode){

	/* if reached end of sub block */
        if(subBlockIndex == subBlockSize){
            newSubBlock(in);
        }

        /*if it is not in the translation table. */
        if(!(newCode < nextCode)){
            stringCodeStack[stackp++] = character;
            translateCode(oldCode);

        } else
            translateCode(newCode);

        character = printString(out);

        /* add it the table */
        if(nextCode <= (pow(2,12) - 1)){

            compressionTable[nextCode].stringCode = oldCode;
            compressionTable[nextCode].characterCode = character;

            debugPrint("Added new dictionary entry:%d {%d = %c,%d = %c}\n",
                       nextCode,
                       oldCode,
                       oldCode,
                       character,
                       character);

            if(nextCode == (pow(2,codeSize) - 1)){
                ++codeSize;
                debugPrint("New code size:%d\n",codeSize);
            }

            nextCode++;
        }
        oldCode = newCode;
        newCode = inputCode(codeSize);
    }

    free(subBlockBytes);
    free(compressionTable);
}

unsigned int inputCode(int codeSize)
{
    unsigned int returnValue;
    static int remainingBits = 8;
    int shift;

    returnValue = 0;
    shift = 0;

    while(codeSize > 0){
        if(remainingBits < codeSize){

            /* read in what's left of the byte */
            returnValue |= (firstNBits(subBlockBytes[subBlockIndex],remainingBits) << shift);
            /* increase the shift */
            shift += remainingBits;
            codeSize -= remainingBits;
            subBlockIndex++;
            remainingBits = 8;

        }else{

            /* if remainingBits > codeSize */
            returnValue |= (firstNBits(subBlockBytes[subBlockIndex],codeSize) << shift);
            subBlockBytes[subBlockIndex] >>= codeSize;
            remainingBits -= codeSize;
            codeSize = 0;

        }
    }

    debugPrint("END:returnValue:%d\n",returnValue);
    return returnValue;
}

unsigned int resetCompressionTable(void)
{
    unsigned int colorTableSize;
    unsigned int nextCode;

    colorTableSize = pow(2,(logicalScreenDescriptor.globalColorTableSize + 1));

    for(nextCode = 0; nextCode < colorTableSize; ++nextCode){

        debugPrint("%d:(%d,%d,%d)\n",nextCode,
                   globalColorTable[nextCode].r,
                   globalColorTable[nextCode].g,
                   globalColorTable[nextCode].b);

        compressionTable[nextCode].characterCode = nextCode;
        compressionTable[nextCode].stringCode = (unsigned int) -1;
    }

    ClearCode = nextCode++;
    EndCode = nextCode++;
    return nextCode;
}


char printString(FILE * out)
{
    char returnValue;

    returnValue = stringCodeStack[stackp - 1];

    while(stackp > 0)
        printColor(stringCodeStack[--stackp],globalColorTable,out);

    return returnValue;
}

void newSubBlock(FILE * in)
{
    BYTE i;
    BYTE b;

    debugPrint("newSubBlock\n");

    subBlockSize = readByte(in);

    debugPrint("subBlockSize:%d\n",subBlockSize);

    /* todo: properly free memory */

    b = readByte(in);

    for(i = 0; i < subBlockSize; ++i){
        debugPrint("read new byte:%d\n",b);
        subBlockBytes[i] = b;
        b = readByte(in);
    }

    subBlockIndex = 0;
}

