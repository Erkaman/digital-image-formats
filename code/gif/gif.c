#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "gif.h"

#include <stdarg.h>

/* Implement command line parsing and -v flag. Clean up debug messages and verbose
 messages. */

int subBlockIndex;

GIFDataSubBlocks imageDataSubBlocks;

tableEntry * compressionTable;

/* Used by all images that do not have a local color table */
GIFColor * globalColorTable;

/* used by the graphics that specifies their own local color tables */
GIFColor * localColorTable;

unsigned long currentColorIndex;

int stringCodeStack[40000];
int stackp;

int remainingBits;

long ClearCode;
long EndCode;

unsigned int * colorIndexTable;

int globalColorTableSize;

/* structures */

GIFImageDescriptor imageDescriptor;

int main(int argc, char *argv[])
{
    FILE * in;
    FILE * out;

    if(argc == 1){
        printf("No file was specified for loading\n");
        printf("Try --help for more information.\n");
        return 1;
    } else
        if(!strcmp("--help",argv[1]))
            printHelp();

    in = fopen(argv[1],"rb");
    assertFileOpened(in);

    out = fopen(argv[2],"wb");
    assertFileOpened(out);

    loadGIF(in, out);

    fclose(in);
    fclose(out);

    return 0;
}

void printHelp(void)
{
    printf("Usage: gif IN OUT\n");
    printf("Dump the information and color data of a GIF file.\n");
    printf("  --help\tDisplay this help message.\n");
}

void loadGIF(FILE * in, FILE * out)
{
    GIFHeader header;
    GIFLogicalScreenDescriptor logicalScreenDescriptor;

    /* load and print the beginning blocks of the GIF format */

    debugPrint("HEADER\n");

    header = loadHeader(in);

    debugPrint("LOGICAL SCREEN DESCRIPTOR\n");

    logicalScreenDescriptor = loadLogicalScreenDescriptor(in);
    globalColorTableSize = logicalScreenDescriptor.globalColorTableSize;
    if(logicalScreenDescriptor.globalColorTableFlag){

	debugPrint("GLOBAL COLOR TABLE\n");
        globalColorTable = loadColorTable(globalColorTableSize,in);
    }

    printHeader(header,out);
    printLogicalScreenDescriptor(logicalScreenDescriptor,out);

    if(logicalScreenDescriptor.globalColorTableFlag){
        fprintf(out,"* Global Color Table:\n");
        printColorTable(globalColorTable,globalColorTableSize,out);
    }

    /* load and print the image data of a GIF image. */
    loadImageData(in,out);

    if(logicalScreenDescriptor.globalColorTableFlag){
        free(globalColorTable);
    }

}

GIFColor * loadColorTable(int colorTableSize,FILE * in)
{
    int realGlobalColorTableSize;
    int i;
    GIFColor * colorTable;

    realGlobalColorTableSize = pow(2.0,1.0 + colorTableSize);

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
            debugPrint("EXTENSION INTRODUCER\n");
            loadExtension(in,out);
            break;
        case IMAGE_SEPARATOR:
            debugPrint("IMAGE SEPARATOR\n");


	    debugPrint("IMAGE DESCRIPTOR\n");
            loadImageDescriptor(in);
            printImageDescriptor(out);

            if(imageDescriptor.localColorTableFlag){

		debugPrint("LOCAL COLOR TABLE\n");

                localColorTable = loadColorTable(imageDescriptor.localColorTableSize,in);

                fprintf(out,"* Local Color Table:\n");

                printColorTable(localColorTable,imageDescriptor.localColorTableSize,out);
            }

            /* perform the decompression of the image data. */
            /* allocate index table */
            colorIndexTable = (unsigned int *)malloc(sizeof(unsigned int) *
                                                     imageDescriptor.imageWidth
                                                     * imageDescriptor.imageHeight);

	    debugPrint("IMAGE COLOR DATA\n");

            loadImageColorData(in);

            fprintf(out,"* Image Color Data:\n");

            printImageColorData(out);

            if(imageDescriptor.localColorTableFlag)
                free(localColorTable);

            free(colorIndexTable);

            localColorTable = NULL;
            colorIndexTable = NULL;

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
    GIFCommentExtension comment;
    GIFPlainTextExtension plainText;

    extensionLabel = readByte(in);

    switch(extensionLabel){
    case GRAPHIC_CONTROL_LABEL:
        debugPrint("GRAPHIC CONTROL LABEL\n");
        graphicControl = loadGraphicControl(in);
        printGraphicControl(graphicControl,out);
        break;
    case COMMENT_LABEL:
        debugPrint("COMMENT LABEL\n");
        comment = loadCommentExtension(in);

        printCommentExtension(comment,out);

        break;
    case APPLICATION_EXTENSION_LABEL:
        debugPrint("APPLICATION EXTENSION LABEL\n");
        applicationExtension = loadApplicationExtension(in);
        printApplicationExtension(applicationExtension,out);
        break;
    case PLAIN_TEXT_LABEL:
        debugPrint("PLAIN TEXT LABEL\n");

        plainText = loadPlainTextExtension(in);

        printPlainTextExtension(plainText,out);

        break;
    }
}

void loadImageColorData(FILE * in)
{
    long codeSize;
    long oldCode;
    long newCode;
    long character;
    long nextCode;
    int InitialCodeSize;

    stackp = 0;

    /* for the inputCode function */

    /* Reset these values for the current sub-image. */
    remainingBits = 8;
    currentColorIndex = 0;
    subBlockIndex = 0;

    /* do this at the beginning of the program? */
    compressionTable = (tableEntry *)malloc(sizeof(tableEntry) * pow(2,12));

    InitialCodeSize = readByte(in) + 1;
    codeSize = InitialCodeSize;

    imageDataSubBlocks = readDataSubBlocks(in);

    /* Skip the clear code. */
    inputCode(codeSize);
    nextCode = resetCompressionTable();

    oldCode = inputCode(codeSize);

    colorIndexTable[currentColorIndex++] = oldCode;

    /* needed? */
    character = oldCode;
    newCode = inputCode(codeSize);

    while(newCode != EndCode){

        /* handle clear codes. */
        if(newCode == ClearCode){

            free(compressionTable);
            compressionTable = (tableEntry *)malloc(sizeof(tableEntry) * pow(2,12));
            debugPrint("Clear Code\n");

            nextCode = resetCompressionTable();
            codeSize = InitialCodeSize;
            oldCode = inputCode(codeSize);
            colorIndexTable[currentColorIndex++] = oldCode;

            newCode = inputCode(codeSize);
        }

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

/*            debugPrint("Added new dictionary entry:%d {%d = %c,%d = %c}\n",
                       nextCode,
                       oldCode,
                       oldCode,
                       character,
                       character);*/

            if(nextCode == (pow(2,codeSize) - 1) &&
               codeSize != 12 ){

                ++codeSize;
/*                debugPrint("New code size:%d\n",codeSize);*/
            }

            nextCode++;
        }
        oldCode = newCode;
        newCode = inputCode(codeSize);
    }

    free(imageDataSubBlocks.data);
    imageDataSubBlocks.data = NULL;

    free(compressionTable);
    compressionTable = NULL;
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

    while(stackp > 0){
        colorIndexTable[currentColorIndex++] = stringCodeStack[--stackp];
/*        debugPrint("outputted color:%d\n",stringCodeStack[stackp]); */
    }

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

	    /* the data in the current byte are not enough bits of data. Reads in what's
	     remaining and read a new byte. */

            /* read in what's left of the current byte */
            returnValue |=
                (firstNBits(imageDataSubBlocks.data[subBlockIndex],remainingBits) << shift);
            /* increase the shift */
            shift += remainingBits;
            codeSize -= remainingBits;
            subBlockIndex++;
            remainingBits = 8;

        }else{
            /* if remainingBits > codeSize */
	    /* Enough bits of data can be read from the current byte.
	       Read in enough data and bitwise shift the data to the right to
	       get rid of the bytes read in. */

            returnValue |=
		(firstNBits(imageDataSubBlocks.data[subBlockIndex],codeSize) << shift);

            imageDataSubBlocks.data[subBlockIndex] >>= codeSize;
            remainingBits -= codeSize;

	    /* enough bits of data has been read in. This line thus causes the loop to terminate
	       and causes the functions to return. */
            codeSize = 0;
        }
    }

/*    debugPrint("END:returnValue:%d\n",returnValue); */
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
        for(r = currentBlocksize; r > 0; r--){
            subBlocks.data[i++] = readByte(in);
/*            debugPrint("%d:%X\n",subBlocks.data[i-1],subBlocks.data[i-1]); */
        }

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

    free(applicationExtension.applicationData.data);
    applicationExtension.applicationData.data = NULL;
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

    debugPrint("%d:%x",packedFields,packedFields);

    graphicControl.reserved = ((packedFields & (5 << 7)) >> 5);
    graphicControl.disposalMethod = ((packedFields & (3 << 2)) >> 2);
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

GIFHeader loadHeader(FILE * in)
{
    GIFHeader header;

    readStr(in,3,header.signature);
    header.signature[3] = '\0';

    readStr(in,3,header.version);
    header.version[3] = '\0';

    return header;
}

GIFCommentExtension loadCommentExtension(FILE * in)
{
    GIFCommentExtension comment;
    GIFDataSubBlocks commentData;

    comment.extensionIntroducer = EXTENSION_INTRODUCER;
    comment.commentLabel = COMMENT_LABEL;

    commentData = readDataSubBlocks(in);
    comment.commentData = subBlocksDataToString(commentData);

    free(commentData.data);

    comment.blockTerminator = BLOCK_TERMINATOR;

    return comment;
}

void printCommentExtension(GIFCommentExtension comment,FILE * out)
{
    fprintf(out,"* Comment Extension\n");

    fprintf(out,"Extension Introducer: %X\n",comment.extensionIntroducer);
    fprintf(out,"Comment Label: %X\n",comment.commentLabel);

    fprintf(out,"Comment Data:\n %s\n",comment.commentData);

    fprintf(out,"Block Terminator: %d\n",comment.blockTerminator);

    free(comment.commentData);
}

char * subBlocksDataToString(GIFDataSubBlocks subBlocks)
{
    char * str;
    unsigned int i;

    str = (char *)malloc(sizeof(char) * (subBlocks.size + 1));

    for(i = 0; i < subBlocks.size; ++i)
        str[i] = (char)subBlocks.data[i];

    str[i] = '\0';
    return str;
}

GIFPlainTextExtension loadPlainTextExtension(FILE * in)
{
    GIFPlainTextExtension plainText;
    GIFDataSubBlocks plainTextData;

    plainText.extensionIntroducer = EXTENSION_INTRODUCER;
    plainText.plainTextLabel = PLAIN_TEXT_LABEL;

    plainText.blockSize = readByte(in);

    plainText.textGridLeftPosition = readUnsigned(in);
    plainText.textGridTopPosition = readUnsigned(in);

    plainText.textGridWidth = readUnsigned(in);
    plainText.textGridHeight = readUnsigned(in);

    plainText.characterCellWidth = readByte(in);
    plainText.characterCellHeight = readByte(in);

    plainText.textForegroundColorIndex = readByte(in);
    plainText.textBackgroundColorIndex = readByte(in);

    plainTextData = readDataSubBlocks(in);
    plainText.plainTextData = subBlocksDataToString(plainTextData);

    free(plainTextData.data);

    plainText.blockTerminator = BLOCK_TERMINATOR;

    return plainText;
}

void printPlainTextExtension(GIFPlainTextExtension plainText,FILE * out)
{
    fprintf(out,"* Plain Text Extension\n");

    fprintf(out,"Extension Introducer: %X\n",plainText.extensionIntroducer);
    fprintf(out,"Plain Text Label: %X\n",plainText.plainTextLabel);

    fprintf(out,"Block size: %d\n",plainText.blockSize);

    fprintf(out,"Text Grid Left Position: %d\n",plainText.textGridLeftPosition);
    fprintf(out,"Text Grid Top Position: %d\n",plainText.textGridTopPosition);

    fprintf(out,"Text Grid Width: %d\n",plainText.textGridWidth);
    fprintf(out,"Text Grid Height: %d\n",plainText.textGridHeight);

    fprintf(out,"Character Cell Width: %d\n",plainText.characterCellWidth);
    fprintf(out,"Character Cell Height: %d\n",plainText.characterCellHeight);

    fprintf(out,"Text Foreground Color Index: %d\n",plainText.textForegroundColorIndex);
    fprintf(out,"Text Background Color Index: %d\n",plainText.textBackgroundColorIndex);

    fprintf(out,"Plain Text Data:\n %s\n",plainText.plainTextData);

    fprintf(out,"Block Terminator: %d\n",plainText.blockTerminator);

    free(plainText.plainTextData);
}

UNSIGNED readUnsigned(FILE * fp)
{
    UNSIGNED s;
    fread(&s,sizeof(UNSIGNED),1,fp);
    return s;
}
