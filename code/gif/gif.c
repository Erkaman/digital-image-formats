#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "util.h"
#include "gif.h"
#include <stdarg.h>

/*
  TODODODODODODODODDO

  fix loading the untitled.gif image from paint. Test different sizes of pure white images from paint.

*/

int subBlockIndex;

GIFDataSubBlocks imageDataSubBlocks;

tableEntry * compressionTable;

/* Used by all images that do not have a local color table */
GIFColor * globalColorTable;

/* used by the graphics that specifies their own local color tables */
GIFColor * localColorTable;

/* use a flag to specify whether the current image has local color table or not,
   else use the global color table */

unsigned int currentColorIndex;

int stringCodeStack[40000];
int stackp;

int remainingBits;

unsigned int ClearCode;
unsigned int EndCode;

unsigned int * colorIndexTable;

BYTE globalColorTableSize;

/* structures */

GIFImageDescriptor imageDescriptor;

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
    GIFHeader header;
    GIFLogicalScreenDescriptor logicalScreenDescriptor;
    FILE * in;
    FILE * out;

    in = fopen(file,"rb");
    assertFileOpened(in);

    file = changeExtension(file,"dmp");
    out = fopen(file,"wb");
    assertFileOpened(out);

    /* load and print the beginning blocks of the GIF format */


    header = loadHeader(in);
    logicalScreenDescriptor = loadLogicalScreenDescriptor(in);
    globalColorTableSize = logicalScreenDescriptor.globalColorTableSize;
    if(logicalScreenDescriptor.globalColorTableFlag)
        globalColorTable = loadColorTable(globalColorTableSize,in);


    printHeader(header,out);
    printLogicalScreenDescriptor(logicalScreenDescriptor,out);

    if(logicalScreenDescriptor.globalColorTableFlag){
        fprintf(out,"* Global Color Table:\n");
        printColorTable(globalColorTable,globalColorTableSize,out);
    }


    /* load and print the image data of a GIF image. */
    loadImageData(in,out);

    fclose(in);
    fclose(out);
    free(file);
}

GIFHeader loadHeader(FILE * in)
{
    GIFHeader header;

    readStr(in,3,header.signature);
    header.signature[3] = '\0';

    readStr(in,3,header.version);
    header.version[3] = '\0';

    return header;
}

GIFLogicalScreenDescriptor loadLogicalScreenDescriptor(FILE * in)
{
    GIFLogicalScreenDescriptor logicalScreenDescriptor;
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

    return logicalScreenDescriptor;
}

void printHeader(GIFHeader header,FILE * out)
{
    fprintf(out,"* Header:\n");
    fprintf(out,"Signature:%s\n",header.signature);
    fprintf(out,"Version:%s\n",header.version);
}

void printLogicalScreenDescriptor(
    GIFLogicalScreenDescriptor logicalScreenDescriptor,
    FILE * out)

{
    fprintf(out,"* Logical Screen Descriptor:\n");

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

GIFColor * loadColorTable(int colorTableSize,FILE * in)
{
    int realGlobalColorTableSize;
    int i;
    GIFColor * colorTable;

    realGlobalColorTableSize = pow(2,1 + colorTableSize);

    colorTable = (GIFColor *) malloc(sizeof(GIFColor) * realGlobalColorTableSize);

    for(i = 0; i <  realGlobalColorTableSize; ++i){
        colorTable[i].r = readByte(in);
        colorTable[i].g = readByte(in);
        colorTable[i].b = readByte(in);
    }

    return colorTable;
}

void printColorTable(GIFColor * colorTable,int colorTableSize,FILE * out)
{
    int realGlobalColorTableSize;
    int i;

    realGlobalColorTableSize = pow(2,1 + colorTableSize);

    for(i = 0; i <  realGlobalColorTableSize; ++i)
        printTableColor(i,colorTable,out);
}

void printTableColor(int index,GIFColor * colorTable,FILE * out)
{
    GIFColor color = colorTable[index];

    fprintf(out,"%d:(%d,%d,%d)\n",index,color.r,color.g,color.b);
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

            if(imageDescriptor.localColorTableFlag){

                localColorTable = loadColorTable(imageDescriptor.localColorTableSize,in);

                fprintf(out,"* Local Color Table:\n");

                printColorTable(localColorTable,imageDescriptor.localColorTableSize,out);
            }


            /* perform the decompression of the image data. */
            /* allocate index table */
            colorIndexTable = (unsigned int *)malloc(sizeof(unsigned int) *
                                                     imageDescriptor.imageWidth
                                                     * imageDescriptor.imageHeight);
            loadImageColorData(in);

	    fprintf(out,"* Image Color Data:\n");

            printImageColorData(out);

            free(colorIndexTable);
            break;
        }

        introducer = readByte(in);
    }
}

void loadExtension(FILE * in,FILE * out)
{
    /* Identifies the extension type. */
    BYTE extensionLabel;
    GIFApplicationExtension applicationExtension;
    GIFGraphicControl graphicControl;

    extensionLabel = readByte(in);

    switch(extensionLabel){
    case GRAPHIC_CONTROL_LABEL:
        debugPrint("GRAPHIC_CONTROL_LABEL\n");
        graphicControl = loadGraphicControl(in);
        printGraphicControl(graphicControl,out);
        break;
    case COMMENT_LABEL:
        debugPrint("COMMENT_LABEL\n");
        break;
    case APPLICATION_EXTENSION_LABEL:
        debugPrint("APPLICATION_EXTENSION_LABEL\n");
        applicationExtension = loadApplicationExtension(in);
        printApplicationExtension(applicationExtension,out);
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
    fprintf(out,"* Image Descriptor:\n");

    fprintf(out,"Image Separator:%X\n",imageDescriptor.imageSeparator);

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

GIFGraphicControl loadGraphicControl(FILE * in)
{
    GIFGraphicControl graphicControl;
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

    return graphicControl;
}

void printGraphicControl(GIFGraphicControl graphicControl,FILE * out)
{
    fprintf(out,"* Graphic Control\n");

    fprintf(out,"Extension Introducer: %X\n",graphicControl.extensionIntroducer);
    fprintf(out,"Graphics Control Label: %X\n",graphicControl.graphicControlLabel);
    fprintf(out,"Block Size: %d\n",graphicControl.blockSize);

    fprintf(out,"Reserved: %d\n",graphicControl.reserved);


    printDisposalMethod(graphicControl,out);

    fprintf(out,"User Input Flag: %d\n",graphicControl.userInputFlag);
    fprintf(out,"Transparency Flag: %d\n",graphicControl.transparencyFlag);

    fprintf(out,"Delay Time: %d\n",graphicControl.delayTime);
    fprintf(out,"Transparency Index: %d\n",graphicControl.transparencyIndex);
    fprintf(out,"Block Terminator: %d\n",graphicControl.blockTerminator);
}

void printDisposalMethod(GIFGraphicControl graphicControl,FILE * out)
{
    fprintf(out,"Disposal Method: %d(",graphicControl.disposalMethod);
    switch(graphicControl.disposalMethod){
    case 0:
        fprintf(out,"No disposal specified");
        break;
    case 1:
        fprintf(out,"Do not dispose");
        break;
    case 2:
        fprintf(out,"Restore to background color");
        break;
    case 3:
        fprintf(out,"Restore to previous");
        break;
    default:
        fprintf(out,"Undefined.");
    }

    fprintf(out,")\n");
}


void loadImageColorData(FILE * in)
{
    int codeSize;
    unsigned int oldCode;
    unsigned int newCode;
    int character;
    unsigned int nextCode;

    stackp = 0;

    /* for the inputCode function */
    remainingBits = 8;
    currentColorIndex = 0;
    subBlockIndex = 0;

    /* do this at the beginning of the program? */
    compressionTable = (tableEntry *)malloc(sizeof(tableEntry) * pow(2,12));

    codeSize = readByte(in) + 1;

    imageDataSubBlocks = readDataSubBlocks(in);

    /* Skip the clear code. */
    inputCode(codeSize);
    nextCode = resetCompressionTable();

    oldCode = inputCode(codeSize);

    colorIndexTable[currentColorIndex++] = oldCode;

    character = oldCode;
    newCode = inputCode(codeSize);

    /* TOO: handle clear codes. */
    while(newCode != EndCode){

        /*if it is not in the translation table. */
        if(!(newCode < nextCode)){
            stringCodeStack[stackp++] = character;
            translateCode(oldCode);
        } else
            translateCode(newCode);

        character = printString();

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

    debugPrint("STOP\n\n");

    freeDataSubBlocks(imageDataSubBlocks);
    free(compressionTable);
}

void translateCode(unsigned int newCode)
{
    tableEntry entry;

    entry = compressionTable[newCode];

    while(1){
        stringCodeStack[stackp++] = entry.characterCode;

        if(entry.stringCode == -1)
            break;
        else
            entry = compressionTable[entry.stringCode];
    }
}

/* TODO: rename to outputString */
int printString(void)
{
    int returnValue;

    returnValue = stringCodeStack[stackp - 1];

    while(stackp > 0)
        colorIndexTable[currentColorIndex++] = stringCodeStack[--stackp];

    return returnValue;
}


unsigned int inputCode(int codeSize)
{
    unsigned int returnValue;
    int shift;

    returnValue = 0;
    shift = 0;

    while(codeSize > 0){
        if(remainingBits < codeSize){

            /* read in what's left of the byte */
            returnValue |=
                (firstNBits(imageDataSubBlocks.data[subBlockIndex],remainingBits) << shift);
            /* increase the shift */
            shift += remainingBits;
            codeSize -= remainingBits;
            subBlockIndex++;
            remainingBits = 8;

        }else{
            /* if remainingBits > codeSize */
            returnValue |= (firstNBits(imageDataSubBlocks.data[subBlockIndex],codeSize) << shift);
            imageDataSubBlocks.data[subBlockIndex] >>= codeSize;
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

    if(imageDescriptor.localColorTableFlag)
        colorTableSize = pow(2,imageDescriptor.localColorTableSize + 1);
    else
        colorTableSize = pow(2,globalColorTableSize + 1);

    for(nextCode = 0; nextCode < colorTableSize; ++nextCode){
        compressionTable[nextCode].characterCode = nextCode;
        compressionTable[nextCode].stringCode = -1;
    }

    ClearCode = nextCode++;
    EndCode = nextCode++;
    return nextCode;
}

GIFApplicationExtension loadApplicationExtension(FILE * in)
{
    GIFApplicationExtension applicationExtension;

    applicationExtension.extensionIntroducer = EXTENSION_INTRODUCER;
    applicationExtension.extensionLabel = APPLICATION_EXTENSION_LABEL;

    applicationExtension.blockSize = readByte(in);

    readStr(in,8,applicationExtension.applicationIdentifier);
    applicationExtension.applicationIdentifier[8] = '\0';

    readStr(in,3,applicationExtension.applicationAuthenticationCode);
    applicationExtension.applicationAuthenticationCode[3] = '\0';

    applicationExtension.applicationData = readDataSubBlocks(in);

    applicationExtension.blockTerminator = readByte(in);

    return applicationExtension;
}

void printApplicationExtension(
    GIFApplicationExtension applicationExtension,
    FILE * out)
{
    fprintf(out,"* Application Extension\n");

    fprintf(out,"Extension Introducer:%X\n",applicationExtension.extensionIntroducer);
    fprintf(out,"Extension Label:%X\n",applicationExtension.extensionLabel);

    fprintf(out,"Block Size:%d\n",applicationExtension.blockSize);

    fprintf(out,"Application Identifier:%s\n",applicationExtension.applicationIdentifier);

    fprintf(
        out,
        "Application Authentication Code:%s\n",
        applicationExtension.applicationAuthenticationCode);

    fprintf(out,"Application Data:\n");

    printDataSubBlocks(
        out,
        applicationExtension.applicationData);

    fprintf(out,"Block Terminator:%d\n",applicationExtension.blockTerminator);

    freeDataSubBlocks(applicationExtension.applicationData);
}

void freeDataSubBlocks(GIFDataSubBlocks dataSubBlocks)
{
    free(dataSubBlocks.data);
}

void printDataSubBlocks(FILE * out,GIFDataSubBlocks subBlocks)
{
    printBytes(out,subBlocks.size,subBlocks.data);
}

void printBytes(FILE * out,size_t size,BYTE * bytes)
{
    size_t i;

    for(i = 0; i < size; ++i)
        if(i < (size - 1))
            fprintf(out,"%d,",bytes[i]);
        else
            fprintf(out,"%d",bytes[i]);

    fprintf(out,"\n");
}

/* TODO: handle sub-blocks larger than one */
GIFDataSubBlocks readDataSubBlocks(FILE * in)
{
    GIFDataSubBlocks subBlocks;
    BYTE currentBlocksize;
    fpos_t startPos;
    unsigned long i;
    BYTE r;

    subBlocks.size = 0;

    fgetpos(in,&startPos);

    /* Calculate the length of the data*/
    currentBlocksize = readByte(in);

    do{
        subBlocks.size += currentBlocksize;
        fseek(in,currentBlocksize,SEEK_CUR);
        currentBlocksize = readByte(in);

    }while(currentBlocksize != 0);

    /* Allocate enough memory to hold all the data */
    subBlocks.data = (BYTE *) malloc(sizeof(BYTE) * subBlocks.size);

    fsetpos(in,&startPos);

    i = 0;

    /* read the data */
    currentBlocksize = readByte(in);
    do{
        for(r = currentBlocksize; r > 0; r--)
            subBlocks.data[i++] = readByte(in);

        currentBlocksize = readByte(in);
    }while(currentBlocksize != 0);

    return subBlocks;
}

/* pass the table used? */
void printImageColorData(FILE * out)
{
    UNSIGNED width;
    UNSIGNED height;
    unsigned long size;
    GIFColor color;
    unsigned int i;
    UNSIGNED col;
    UNSIGNED row;
    GIFColor * colorTable;

    if(imageDescriptor.localColorTableFlag){
        colorTable = localColorTable;
    } else{
        colorTable = globalColorTable;
    }

    width = imageDescriptor.imageWidth;
    height = imageDescriptor.imageHeight;
    size = width * height;
    col = 0;
    row = 0;

    for(i = 0;i < size; ++i){
        color = colorTable[colorIndexTable[i]];

        if(col == 0)
            fprintf(out,"Row %d:\n",row);

        fprintf(out,"(%d,%d,%d)",color.r,color.g,color.b);

        ++col;

        if(col == width){
            fprintf(out,"\n\n");
            col = 0;
            ++row;
        }
    }
}

void debugPrint(const char * format, ...)
{
    va_list vl;

    if(DEBUG){
        va_start(vl, format);
        vprintf(format, vl);
        va_end(vl);
    }
}
