#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdarg.h>
#include "gif.h"

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

    /* read the image info; things like color table and headers */
    readImageInfo(in);

    fprintf(out,"* Image info:\n");

    printImageInfo(out);

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
    GIFColor color;
    int realGlobalColorTableSize;
    int i;

    realGlobalColorTableSize = pow(2,1 + logicalScreenDescriptor.globalColorTableSize);

    /* ugly hack to fixes segfault */
    globalColorTable = (GIFColor *) malloc(realGlobalColorTableSize);

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

void printTableColor(int index,GIFColor * colorTable,FILE * out)
{
    unsigned long r,g,b;
    unsigned long color;

    color = colorTable[index];

    /* because the color is stored least significant bit first. */
    b = (color & (0xff << 16)) >> 16;
    g = (color & (0xff << 8)) >> 8;
    r = color & 0xff;

    fprintf(out,"%d:(%lu,%lu,%lu)\n",index,r,g,b);
}

void printColor(int index,GIFColor * colorTable,FILE * out)
{
    static int col = 0;

    unsigned long r,g,b;
    unsigned long color;

    color = colorTable[index];

    /* because the color is stored least significant bit first. */
    b = (color & (0xff << 16)) >> 16;
    g = (color & (0xff << 8)) >> 8;
    r = color & 0xff;

    fprintf(out,"(%lu,%lu,%lu)",r,g,b);

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
            loadExtension(in,out);
            break;
        case IMAGE_SEPARATOR:

            /* if local color table size > 0 load local color table */
            loadImageDescriptor(in);
            printImageDescriptor(out);

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
    int subBlockSize;

    unsigned int oldCode;
    unsigned int newCode;
    char character;

    stackp = 0;

    /* do this at the beginning of the program? */
    compressionTable = (tableEntry *)malloc(pow(2,12));

    codeSize = readByte(in) + 1;

    /* figure out how to use this value later */
    subBlockSize = readByte(in);

    /* Skip the clear code. */
    inputCode(codeSize,in);
    resetCompressionTable();

    oldCode = inputCode(codeSize,in);

    printColor(oldCode,globalColorTable,out);

    character = oldCode;
    newCode = inputCode(codeSize,in);

    while(newCode != EndCode){

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

/*            debugPrint("Added new dictionary entry:%d {%d = %c,%d = %c}\n",
                       nextCode,
                       oldCode,
                       oldCode,
                       character,
                       character);*/

            if(nextCode == (pow(2,codeSize) - 1)){
                ++codeSize;
            }

            nextCode++;
        }

        oldCode = newCode;
        newCode = inputCode(codeSize,in);
    }

    /* read and throw away the block terminator. */
    readByte(in);

    free(compressionTable);
}

unsigned int inputCode(int codeSize,FILE *input)
{
    unsigned int returnValue;
    static int inputBitCount=0;
    int inputt;
    static unsigned int inputBitBuffer=0;


    if(inputBitCount < codeSize){

        inputt = getc(input);
/*      debugPrint("hai:%d\n",inputt);
        debugPrint("hai2:%d\n",(inputt << (inputBitCount == 0 ? 0 : codeSize)));*/

        inputBitBuffer = (inputBitBuffer) | (inputt << (inputBitCount));
        inputBitCount += 8;
    }

/*    debugPrint("BEFORE inputBitCount:%d\n",inputBitCount);
      debugPrint("BEFORE inputBitBuffer:%d\n",inputBitBuffer);*/

    returnValue = inputBitBuffer & (~(~0 << codeSize));

    inputBitBuffer >>= codeSize;
    inputBitCount -= codeSize;

/*    debugPrint("AFTER inputBitCount:%d\n",inputBitCount);
      debugPrint("AFTER inputBitBuffer:%d\n",inputBitBuffer);*/


    return returnValue;

/*    inputBitBuffer = inputBitBuffer << (24 - inputBitCount); */

/*    while (inputBitCount <= 24)
      {
      inputBitBuffer |=
      (unsigned int) getc(input) << (inputBitCount);
      inputBitCount += 8;
      }*/

/*    returnValue = inputBitBuffer & ~(~0 << codeSize)
      inputBitBuffer <<= codeSize;
      inputBitCount -= codeSize;*/

}

void resetCompressionTable(void)
{
    unsigned int colorTableSize;

    colorTableSize = pow(2,(logicalScreenDescriptor.globalColorTableSize + 1));

    for(nextCode = 0; nextCode < colorTableSize; ++nextCode){
        compressionTable[nextCode].characterCode = nextCode;
        compressionTable[nextCode].stringCode = (unsigned int) -1;
    }

    ClearCode = nextCode++;
    EndCode = nextCode++;
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

char printString(FILE * out)
{
    char returnValue;

    returnValue = stringCodeStack[stackp - 1];

    while(stackp > 0)
        printColor(stringCodeStack[--stackp],globalColorTable,out);

    return returnValue;
}
