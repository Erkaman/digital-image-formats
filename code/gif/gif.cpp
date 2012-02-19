#include "../io.h"
#include "../bits.h"
#include "../lzw/lzwutil.h"
#include "gif_defs.h"

#include <cstring>
#include <cstdlib>
#include <cmath>
#include <vector>

using std::vector;

void printHelp(void);

void loadGIF(FILE * in, FILE * out);

void loadImageData(FILE * in,FILE * out);

void loadExtension(FILE * in,FILE * out);

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

unsigned int * loadImageColorData(FILE * in);

void printImageColorData(unsigned int * colorIndexTable,FILE * out);


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

GIFDataSubBlocks readDataSubBlocks(FILE * in);

void printDataSubBlocks(GIFDataSubBlocks subBlocks, FILE * out);

void printDisposalMethod(GIFGraphicControl graphicControl,FILE * out);

char * subBlocksDataToString(GIFDataSubBlocks subBlocks);

UNSIGNED readUnsigned(FILE * fp);

unsigned int * uninterlaceColorData(unsigned int * colorIndexTable);

code resetCompressionTable(std::vector<codeStr> & stringTable);

using std::vector;
using std::map;

/* Implement command line parsing and -v flag. Clean up debug messages and verbose
   messages. */

/* Used by all images that do not have a local color table */
GIFColor * globalColorTable;

/* used by the graphics that specifies their own local color tables */
GIFColor * localColorTable;

unsigned long currentColorIndex;

long ClearCode;
long EndCode;

int globalColorTableSize;

GIFImageDescriptor imageDescriptor;

int main(int argc, char *argv[])
{
    FILE * in;
    FILE * out;

    if(argc == 1){
        printf("No file was specified for loading\n");
        printf("Try --help for more information.\n");
        return 1;
    } else{
        ++argv;
        --argc;

        while(1){

            if(!strcmp("--help",*argv)){
                printHelp();
                return 0;
            }
            else if(!strcmp("-v",*argv))
                verbose = 1;
            else
                break;

            ++argv;
            --argc;
        }
    }

    in = fopen(argv[0],"rb");
    assertFileOpened(in);

    out = fopen(argv[1],"wb");
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

    verbosePrint("Loading Header\n");

    header = loadHeader(in);

    verbosePrint("Loading Logical Screen Descriptor\n");

    logicalScreenDescriptor = loadLogicalScreenDescriptor(in);
    globalColorTableSize = logicalScreenDescriptor.globalColorTableSize;
    if(logicalScreenDescriptor.globalColorTableFlag){

        verbosePrint("Loading Global Color Table\n");
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
        colorTable[i].r = getc(in);
        colorTable[i].g = getc(in);
        colorTable[i].b = getc(in);
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

    introducer = getc(in);

    while(introducer != TRAILER){

        switch(introducer){
        case EXTENSION_INTRODUCER:
            verbosePrint("Found Extension Introducer\n");


            loadExtension(in,out);
            break;
        case IMAGE_SEPARATOR:
            verbosePrint("Found Image Separator\n");
            verbosePrint("Loading Image Descriptor\n");
            loadImageDescriptor(in);
            printImageDescriptor(out);

            if(imageDescriptor.localColorTableFlag){

                verbosePrint("Loading Local Color Table\n");

                localColorTable = loadColorTable(imageDescriptor.localColorTableSize,in);

                fprintf(out,"* Local Color Table:\n");

                printColorTable(localColorTable,imageDescriptor.localColorTableSize,out);
            }

            unsigned int * colorIndexTable;

            /* perform the decompression of the image data. */
            /* allocate index table */

            verbosePrint("Loading Image Color Data\n");

            colorIndexTable = loadImageColorData(in);

            if(imageDescriptor.interlaceFlag)
                colorIndexTable = uninterlaceColorData(colorIndexTable);

            fprintf(out,"* Image Color Data:\n");

            printImageColorData(colorIndexTable, out);

            if(imageDescriptor.localColorTableFlag)
                free(localColorTable);

            free(colorIndexTable);

            localColorTable = NULL;
            colorIndexTable = NULL;

            break;
        }

	verbosePrint("%d\n", introducer);

        introducer = getc(in);
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

    extensionLabel = getc(in);

    switch(extensionLabel){
    case GRAPHIC_CONTROL_LABEL:
        verbosePrint("Loading Graphic Control\n");
        graphicControl = loadGraphicControl(in);
        printGraphicControl(graphicControl,out);
        break;
    case COMMENT_LABEL:
        verbosePrint("Loading Comment Label\n");
        comment = loadCommentExtension(in);

        printCommentExtension(comment,out);

        break;
    case APPLICATION_EXTENSION_LABEL:
        verbosePrint("Loading Application Extension\n");
        applicationExtension = loadApplicationExtension(in);
        printApplicationExtension(applicationExtension,out);
        break;
    case PLAIN_TEXT_LABEL:
        verbosePrint("Loading Plain Text Extension\n");

        plainText = loadPlainTextExtension(in);

        printPlainTextExtension(plainText,out);

        break;
    }
}

unsigned int * loadImageColorData(FILE * in)
{
    code oldCode;
    code newCode;
    code nextCode;

    codeStr str;
    code character;

    code InitialCodeSize;
    code codeSize;

    vector<codeStr> stringTable;

    GIFDataSubBlocks imageDataSubBlocks;

    unsigned int * colorIndexTable;

    colorIndexTable = (unsigned int *)malloc(sizeof(unsigned int) *
                                             imageDescriptor.imageWidth
                                             * imageDescriptor.imageHeight);

    currentColorIndex = 0;

    InitialCodeSize = getc(in) + 1;
    printf("InitialCodeSize:%ld\n", InitialCodeSize);

    codeSize = InitialCodeSize;

    imageDataSubBlocks = readDataSubBlocks(in);

    BitIterReader inbits(imageDataSubBlocks.begin(),LSBF);

    /* Skip the clear code. */
    printf("%d\n", inbits.readBits(codeSize));

    nextCode = resetCompressionTable(stringTable);

    oldCode = inbits.readBits(codeSize);

    colorIndexTable[currentColorIndex++] = oldCode;

    character = oldCode;
    newCode = inbits.readBits(codeSize);


    /* Due to a bug, the size of the image size array is surpused
     and the reading of the image terminates delayed. */
    while(newCode != EndCode){
        /* handle clear codes. */
        if(newCode == ClearCode){
	    printf("clear code\n");
            /* free string table*/
            stringTable.clear();
            nextCode = resetCompressionTable(stringTable);
            codeSize = InitialCodeSize;
            oldCode = inbits.readBits(codeSize);
            colorIndexTable[currentColorIndex++] = oldCode;

            newCode = inbits.readBits(codeSize);
        }

        /*if it is not in the translation table. */
        if(!(newCode < nextCode)){
            str.first = oldCode;
            str.second = character;
        } else{
            str = stringTable[newCode];
        }

        vector<code> outStr = outputCodes(str,stringTable);
        character = outStr[0];

        for(size_t i = 0; i < outStr.size(); ++i){

/*	    printf("i:%ld\n", currentColorIndex); */
            colorIndexTable[currentColorIndex++] =
		outStr[i];
	}

/*	if(currentColorIndex >= (imageDescriptor.imageWidth * imageDescriptor.imageHeight))
	    break;*/

        /* add it the table */
        if(nextCode <= (pow(2,12) - 1)){

            codeStr newEntry;

            newEntry.first = oldCode;
            newEntry.second = character;

            stringTable.push_back(newEntry);

            if(nextCode == (pow(2,codeSize) - 1) &&
               codeSize != 12 ){
                ++codeSize;
            }

            nextCode++;
        }
        oldCode = newCode;
        newCode = inbits.readBits(codeSize);
    }

    return colorIndexTable;
}

code resetCompressionTable(vector<codeStr> & stringTable)
{
    code colorTableSize;
    code nextCode;

    if(imageDescriptor.localColorTableFlag)
        colorTableSize = pow(2,imageDescriptor.localColorTableSize + 1);
    else
        colorTableSize = pow(2,globalColorTableSize + 1);

    for(nextCode = 0; nextCode < colorTableSize; ++nextCode){
        codeStr cs;
        cs.first = nextCode;
        cs.second = emptyCh;
        stringTable.push_back(cs);
    }

    codeStr emptyCode;

    emptyCode.first = emptyCh;
    emptyCode.second = emptyCh;

    stringTable.push_back(emptyCode);
    stringTable.push_back(emptyCode);

    ClearCode = nextCode++;
    EndCode = nextCode++;
    printf("clearCode:%ld\n", ClearCode);
    printf("EndCode:%ld\n", EndCode);
    return nextCode;
}

void printDataSubBlocks(GIFDataSubBlocks subBlocks, FILE * out)
{
    size_t i;

    for(i = 0; i < subBlocks.size(); ++i)
        if(i < (subBlocks.size() - 1))
            fprintf(out,"%d,",subBlocks[i]);
        else
            fprintf(out,"%d",subBlocks[i]);

    fprintf(out,"\n");

}

/* TODO: handle sub-blocks larger than one */
GIFDataSubBlocks readDataSubBlocks(FILE * in)
{
    GIFDataSubBlocks subBlocks;
    BYTE currentBlocksize;
    BYTE r;

    /* read the data */
    currentBlocksize = getc(in);
    do{
        for(r = currentBlocksize; r > 0; r--)
	    subBlocks.push_back(getc(in));
        currentBlocksize = getc(in);
    }while(currentBlocksize != 0);

    return subBlocks;
}

/* pass the table used? */
void printImageColorData(unsigned int * colorIndexTable,FILE * out)
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

GIFApplicationExtension loadApplicationExtension(FILE * in)
{
    GIFApplicationExtension applicationExtension;

    applicationExtension.extensionIntroducer = EXTENSION_INTRODUCER;
    applicationExtension.extensionLabel = APPLICATION_EXTENSION_LABEL;

    applicationExtension.blockSize = getc(in);

    readStr(in,8,applicationExtension.applicationIdentifier);
    applicationExtension.applicationIdentifier[8] = '\0';

    readStr(in,3,applicationExtension.applicationAuthenticationCode);
    applicationExtension.applicationAuthenticationCode[3] = '\0';

    applicationExtension.applicationData = readDataSubBlocks(in);

    applicationExtension.blockTerminator = getc(in);

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

    printDataSubBlocks(applicationExtension.applicationData, out);

    fprintf(out,"Block Terminator:%d\n",applicationExtension.blockTerminator);
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

    packedFields = getc(in);

    imageDescriptor.localColorTableFlag = getbits(packedFields,7,7);
    imageDescriptor.interlaceFlag = getbits(packedFields,6,6);
    imageDescriptor.sortFlag = getbits(packedFields,5,5);
    imageDescriptor.reserved = getbits(packedFields,3,4);
    imageDescriptor.localColorTableSize = getbits(packedFields,0,2);
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

    graphicControl.blockSize = getc(in);

    packedFields = getc(in);

    graphicControl.reserved = getbits(packedFields,5,7);
    graphicControl.disposalMethod = getbits(packedFields,2,4);
    graphicControl.userInputFlag = getbits(packedFields,1,1);
    graphicControl.transparencyFlag = getbits(packedFields,0,0);

    graphicControl.delayTime = readUnsigned(in);
    graphicControl.transparencyIndex = getc(in);
    graphicControl.blockTerminator = getc(in);

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

    packedFields = getc(in);
    logicalScreenDescriptor.globalColorTableFlag = getbits(packedFields,7,7);
    logicalScreenDescriptor.colorResolution = getbits(packedFields,4,6);
    logicalScreenDescriptor.sortFlag = getbits(packedFields,3,3);
    logicalScreenDescriptor.globalColorTableSize = getbits(packedFields,0,2);

    logicalScreenDescriptor.backgroundColorIndex = getc(in);
    logicalScreenDescriptor.pixelAspectRatio = getc(in);

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

    str = (char *)malloc(sizeof(char) * (subBlocks.size() + 1));

    for(i = 0; i < subBlocks.size(); ++i)
        str[i] = (char)subBlocks[i];

    str[i] = '\0';
    return str;
}

GIFPlainTextExtension loadPlainTextExtension(FILE * in)
{
    GIFPlainTextExtension plainText;
    GIFDataSubBlocks plainTextData;

    plainText.extensionIntroducer = EXTENSION_INTRODUCER;
    plainText.plainTextLabel = PLAIN_TEXT_LABEL;

    plainText.blockSize = getc(in);

    plainText.textGridLeftPosition = readUnsigned(in);
    plainText.textGridTopPosition = readUnsigned(in);

    plainText.textGridWidth = readUnsigned(in);
    plainText.textGridHeight = readUnsigned(in);

    plainText.characterCellWidth = getc(in);
    plainText.characterCellHeight = getc(in);

    plainText.textForegroundColorIndex = getc(in);
    plainText.textBackgroundColorIndex = getc(in);

    plainTextData = readDataSubBlocks(in);
    plainText.plainTextData = subBlocksDataToString(plainTextData);

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

unsigned int * uninterlaceColorData(unsigned int * colorIndexTable)
{
    unsigned int * transfer;
    unsigned int i;
    unsigned int row, col;
    unsigned int height, width;

    int pass;
    unsigned int startingRow[4] = {0, 4, 2, 1};
    unsigned int rowIncrement[4] = {8, 8, 4, 2};

    height = imageDescriptor.imageHeight;
    width = imageDescriptor.imageWidth;

    transfer = (unsigned int *)malloc(sizeof(unsigned int) * height * width);

    row = 0;

    i = 0;

    for(pass = 0; pass < 4; ++pass){
        row = startingRow[pass];

        /* since height is one based and row is zero based this works.*/
        while(row < height){

            for(col = 0;col < width; ++col)
                transfer[row * width + col] = colorIndexTable[i++];

            row += rowIncrement[pass];
        }
    }

    free(colorIndexTable);
    return transfer;
}
