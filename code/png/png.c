#include "png.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <math.h>
#include "../print_funcs.h"
#include "../data_stream.h"

void * copyByte(void * vptr)
{
    BYTE * copy;
    BYTE b;

    b = *(BYTE *)vptr;

    copy = malloc(sizeof(BYTE));
    *copy = b;

    return copy;
}

void dumpPNG(FILE * in, FILE * out)
{
    PNG_Image image;

    image = loadPNG(in);
    writePNG(image,out);
    freePNG_Image(image);
}

void freePNG_Image(PNG_Image image)
{
    if(image.renderingIntent != NULL)
        free(image.renderingIntent);

    if(image.imageGamma != NULL)
        free(image.imageGamma);

    if(image.timeStamp != NULL)
        free(image.timeStamp);

    if(image.backgroundColor != NULL)
        free(image.backgroundColor);

    if(image.pixelDimensions != NULL)
        free(image.pixelDimensions);

    freeDataList(image.textDataList,1);

}

void loadSignature(BYTE * signature, FILE * in)
{
    fread(signature, sizeof(BYTE),SIGNATURE_LENGTH , in);
}

PNG_Image getEmptyPNG_Image(void)
{
    PNG_Image image;

    image.palette = NULL;
    image.profile = NULL;
    image.chromaticities = NULL;
    image.imageGamma = NULL;
    image.significantBits = NULL;
    image.renderingIntent = NULL;
    image.backgroundColor = NULL;
    image.imageHistogram = NULL;
    image.pixelDimensions = NULL;
    image.timeStamp = NULL;

    image.textDataList = getNewDataList(freeTextualData, copyByte);

    return image;
}

void freeTextualData(void * textualData)
{
    TextualData * data;

    data = (TextualData *) textualData;

    if(data->str != NULL)
        free(data->str);

    free(data);
}

PNG_Image loadPNG(FILE * in)
{
    PNG_Image image;
    Chunk chunk;
    DataStream stream;

    image = getEmptyPNG_Image();

    loadSignature(image.signature, in);

    image.header = loadImageHeader(in);

    while(1){

        chunk = loadChunk(in);

        stream = getNewDataStream(chunk.data, PNG_ENDIAN);

        if(isChunkType(chunk, IEND))
            break;
        else if(isChunkType(chunk, IDAT)){

        }else if(isChunkType(chunk, sRGB))
            image.renderingIntent = loadRenderingIntent(stream);

        else if(isChunkType(chunk, gAMA))
            image.imageGamma = loadImageGamma(stream);

        else if(isChunkType(chunk, pHYs))
            image.pixelDimensions = loadPixelDimensions(stream);

        else if(isChunkType(chunk, tIME))
            image.timeStamp = loadTimeStamp(stream);

        else if(isChunkType(chunk, bKGD))
            image.backgroundColor = loadBackgroundColor(image.header, stream);

        else if(isChunkType(chunk, tEXt))
            addToDataList(
                &image.textDataList,
                loadTextualData(stream, 0, chunk.length));

        else {
            if(!isCriticalChunk(chunk)){
                printWarning("Unknown ancillary chunk %s found, skipping chunk.\n",
                             chunk.type);
            } else{
                printError("Unknown critcal chunk %s found.\n", chunk.type);
/*                exit(1); */
            }
        }

        freeChunk(chunk);
    }

    return image;
}

int isChunkType(Chunk chunk, char * chunkType)
{
    return (!strcmp(chunk.type, chunkType));
}

void freeChunk(Chunk chunk)
{
    freeDataList(chunk.data,1);
}

void writePNG(PNG_Image image, FILE * out)
{
    writeSignature(image.signature, out);
    writeHeader(image.header, out);

    writeRenderingIntent(image.renderingIntent, out);
    writeImageGamma(image.imageGamma, out);
    writePixelDimensions(image.pixelDimensions, out);
    writeTimeStamp(image.timeStamp, out);
    writeBackgroundColor(image.header, image.backgroundColor, out);

    writeTextDataList(image.textDataList, out);
}

void writeRenderingIntent(BYTE * renderingIntent, FILE * out)
{
    if(renderingIntent != NULL){
        fprintf(out,"Rendering Intent: ");

        switch(*renderingIntent){
        case PERCEPTUAL_RENDERING_INTENT:
            fprintf(out, "Perceptual");
            break;
        case RELATIVE_COLORIMETRIC_RENDERING_INTENT:
            fprintf(out, "Relative Colorimetric");
            break;
        case SATURATION_RENDERING_INTENT:
            fprintf(out, "Saturation");
            break;
        case ABSOLUTE_COLORIMETRIC_RENDERING_INTENT:
            fprintf(out, "Absolute colorimetric");
            break;
        }

        fprintf(out, "\n");
    }
}

void writeHeader(ImageHeader header, FILE * out)
{
    fprintf(out, "PNG Image Header:\n");

    fprintf(out, "Image width: %u\n", header.width);
    fprintf(out, "Image height: %u\n", header.height);

    fprintf(out, "Bit Depth: %u bits\n", header.bitDepth);
    fprintf(out, "Color type: ");

    switch(header.colorType){
    case GREYSCALE_COLOR:
        fprintf(out, "Greyscale");
        break;
    case TRUECOLOR_COLOR:
        fprintf(out, "Truecolor");
        break;
    case INDEXED_COLOR:
        fprintf(out, "Indexed Color");
        break;
    case GREYSCALE_ALPHA_COLOR:
        fprintf(out, "Greyscale Alpha Color");
        break;
    case TRUECOLOR_ALPHA_COLOR:
        fprintf(out, "Truecolor Alpha Color");
        break;
    }

    fprintf(out, "(%u)\n",header.colorType);

    fprintf(out, "Compression Method: %u(always deflate if 0)\n", header.compressionMethod);
    fprintf(out, "Filter Method: %u(always 0, which is default filtering)\n", header.filterMethod);

    fprintf(out, "Interlace Method:");

    if(header.interlaceMethod == NO_INTERLACE)
        fprintf(out,"No interlacing");
    else if(header.interlaceMethod == ADAM7_INTERLACE)
        fprintf(out,"Adam7 interlacing");

    fprintf(out, "(%u)\n",header.interlaceMethod);
}

void writeSignature(BYTE * signature, FILE * out)
{
    size_t i;

    fprintf(out, "PNG Signature:\n");
    for(i = 0; i < SIGNATURE_LENGTH; ++i)
        if(i != (SIGNATURE_LENGTH - 1))
            fprintf(out, "%d ", signature[i]);
        else
            fprintf(out, "%d\n", signature[i]);
}

Chunk loadChunk(FILE * in)
{
    Chunk chunk;

    verbosePrint("Loading chunk...\n");

    fread(&chunk.length, sizeof(INT32), 1, in);
    chunk.length = htonl(chunk.length);

    verbosePrint("Chunk length: % ld\n", chunk.length);

    fread(chunk.type, sizeof(char), 4, in);
    chunk.type[4] = '\0';
    verbosePrint("Chunk Type: %s\n", chunk.type);

    /* TODO: Remember to free this memory! */
    chunk.data = readBytes(chunk.length, in);

    verbosePrint("data:\n");
    printDataList(chunk.data, printByte);

    fread(&chunk.CRC, sizeof(INT32), 1, in);
    chunk.CRC = htonl(chunk.CRC);
    verbosePrint("Chunk CRC: %ld\n", chunk.CRC);

    validateCRC(chunk);

    return chunk;
}

DataList readBytes(size_t count, FILE * in)
{
    DataList list;
    size_t i;

    list = getNewDataList(NULL, copyByte);

    for(i = 0; i < count; ++i)
        addByteToDataList(&list, (BYTE)getc(in));

    return list;
}

/* Some kind of copying routine is needed! */
void validateCRC(Chunk chunk)
{
    DataList checkData;

    INT32 calcCRC;
    BYTE b;
    size_t i;

    checkData = getNewDataList(NULL, copyByte);

    /* Convert the chunk type string to an array of bytes. */

    for(i = 0; i < 4; ++i){
        b = chunk.type[i];
        addByteToDataList(&checkData, b);
    }

    copyAppendToDataList(&checkData, chunk.data);

    calcCRC = crc32(checkData);

    freeDataList(checkData, 1);

    if(calcCRC != chunk.CRC){
        printError("Chunk has invalid checksum: chunk %d != calc %d\n", chunk.CRC, calcCRC );
        exit(1);
    } else
        verbosePrint("Chunk has valid checksum!\n");

}

int isCriticalChunk(Chunk chunk)
{
    return isupper((BYTE)chunk.type[0]);
}

ImageHeader loadImageHeader(FILE * in)
{
    ImageHeader header;
    Chunk headerChunk;
    DataStream stream;

    headerChunk = loadChunk(in);
    stream = getNewDataStream(headerChunk.data, PNG_ENDIAN);

    header.width = read32BitsNumber(&stream);
    header.height = read32BitsNumber(&stream);
    header.bitDepth = readStreamByte(&stream);
    header.colorType = readStreamByte(&stream);
    header.compressionMethod = readStreamByte(&stream);
    header.filterMethod = readStreamByte(&stream);
    header.interlaceMethod = readStreamByte(&stream);

    freeChunk(headerChunk);

    return header;
}

BYTE * loadRenderingIntent(DataStream stream)
{
    BYTE * b;

    b = malloc(sizeof(BYTE));

    *b = readStreamByte(&stream);

    return b;
}

#define CRC32_POLY 0xEDB88320

unsigned int crc32(DataList data){

    unsigned long reminder = 0xFFFFFFFF; /* standard initial value in CRC32 */
    unsigned long i;
    unsigned long bit;
    BYTE b;

    for(i = 0; i < data.count; ++i){
        b = *(BYTE *)data.list[i];
        reminder ^= b; /* must be zero extended */
        for(bit = 0; bit < 8; bit++)
            if(reminder & 0x01)
                reminder = (reminder >> 1) ^ CRC32_POLY;
            else
                reminder >>= 1;
    }

    return reminder ^ 0xFFFFFFFF;
}

INT32 * loadImageGamma(DataStream stream)
{
    INT32 * gamma;

    gamma = malloc(sizeof(INT32));

    *gamma = read32BitsNumber(&stream);

    return gamma;
}
void writeImageGamma(INT32 * imageGamma, FILE * out)
{
    if(imageGamma != NULL){
        fprintf(out, "Image Gamma: %d / 100000\n", *imageGamma);
    }
}

PixelDimensions * loadPixelDimensions(DataStream stream)
{
    PixelDimensions * pixelDimensions;

    pixelDimensions = malloc(sizeof(PixelDimensions));

    pixelDimensions->x = read32BitsNumber(&stream);
    pixelDimensions->y = read32BitsNumber(&stream);

    pixelDimensions->unitSpecifier = readStreamByte(&stream);

    return pixelDimensions;
}

void writePixelDimensions(PixelDimensions * pixelDimensions, FILE * out)
{
    char unit[] = "Unknown unit";


    if(pixelDimensions != NULL){

        fprintf(out, "Pixel Dimensions:\n");

        if(pixelDimensions->unitSpecifier == METRE_UNIT)
            strcpy(unit,"Metre");

        fprintf(out, "X:%d pixels per %s\n",pixelDimensions->x, unit);
        fprintf(out, "Y:%d pixels per %s\n",pixelDimensions->y, unit);
    }
}

TimeStamp * loadTimeStamp(DataStream stream)
{
    TimeStamp * timeStamp;

    timeStamp = malloc(sizeof(TimeStamp));

    timeStamp->year = read16BitsNumber(&stream);

    timeStamp->month = readStreamByte(&stream);
    timeStamp->day = readStreamByte(&stream);
    timeStamp->hour = readStreamByte(&stream);
    timeStamp->minute = readStreamByte(&stream);
    timeStamp->second = readStreamByte(&stream);

    return timeStamp;
}

void writeTimeStamp(TimeStamp * timeStamp, FILE * out)
{
    if(timeStamp != NULL){
        fprintf(out,"Image last-modification time: ");

        fprintf(out, "%d/%d - %d %02d:%02d:%02d\n",
                timeStamp->day,
                timeStamp->month,
                timeStamp->year,
                timeStamp->hour,
                timeStamp->minute,
                timeStamp->second
            );
    }
}

Color * loadBackgroundColor(ImageHeader header, DataStream stream)
{
    Color * backgroundColor;

    backgroundColor = malloc(sizeof(Color));

    /*

      "The background colour given by the bKGD chunk is not to be
      considered transparent"

      And therefore, the value of the alpha channel will always
      be its maximum value.

    */

    if(header.colorType == GREYSCALE_COLOR){

        backgroundColor->greyscale = read16BitsNumber(&stream);

    } else if(header.colorType == GREYSCALE_ALPHA_COLOR){

        backgroundColor->greyscaleAlpha.greyscale = read16BitsNumber(&stream);

        backgroundColor->greyscaleAlpha.alpha =
            getMaximumChannelValue(header);
    }

/* convert -size 1x1 xc:transparent -fill 'rgba(180, 180, 180, 0.8)' -draw 'rectangle 0,0 1,1' bgbox.png
 */

    return backgroundColor;
}

INT32 getMaximumChannelValue(ImageHeader header)
{
    return pow(2, header.bitDepth) - 1;
}

void addByteToDataList(DataList * list, BYTE b)
{
    BYTE * bp;

    bp = malloc(sizeof(BYTE));
    *bp = b;
    addToDataList(list, bp);
}

void writeBackgroundColor(
    ImageHeader header,
    Color * backgroundColor,
    FILE * out)
{
    if(backgroundColor != NULL){
        fprintf(out,"Background color: ");
        writeColor(header,*backgroundColor, out);
        fprintf(out,"\n");
    }
}

void writeColor(ImageHeader header,
                Color color,
                FILE * out)
{

    if(header.colorType == GREYSCALE_COLOR){

        fprintf(out, "(%d,%d,%d)",
                color.greyscale,
                color.greyscale,
                color.greyscale);

    } else if(header.colorType == GREYSCALE_ALPHA_COLOR){

        fprintf(out, "(%d,%d,%d,%d)",
                color.greyscaleAlpha.greyscale,
                color.greyscaleAlpha.greyscale,
                color.greyscaleAlpha.greyscale,
                color.greyscaleAlpha.alpha);
    }
}

TextualData * loadTextualData(
    DataStream stream,
    int compressed,
    INT32 chunkLength)
{
    TextualData * data;
    char ch;
    int i;
    int textLength;


    data = malloc(sizeof(TextualData));
    data->str = NULL;

    /* Read the keyword. */
    i = 0;
    do{
        ch = (char)readStreamByte(&stream);

        data->keyword[i++] = ch;

    } while(ch != '\0');


    if(compressed){
        /* Uncompress the data.*/
    } else{
        /* Simply read the data.

         */

        textLength = chunkLength - i;

        /* +1 makes space for the null character. */
        data->str = malloc(sizeof(char) * (textLength + 1));

        for(i = 0; i < (textLength); ++i){
	    data->str[i] = (char)readStreamByte(&stream);
	}

	data->str[i] = '\0';
    }

    return data;
}

void writeTextDataList(DataList textDataList, FILE * out)
{
    size_t i;
    TextualData * data;
    if(textDataList.count > 0){
        verbosePrint("holy shit:%d", textDataList.count);

        for(i = 0; i < textDataList.count; ++i){
            fprintf(out, "Textual data %ld:\n", i+1);
            data = (TextualData *)textDataList.list[i];
            fprintf(out, "Keyword: %s\n", data->keyword);

            fprintf(out, "Text: %s\n", data->str);

        }
    }
}
