#include "png.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "../print_funcs.h"
#include "../data_stream.h"

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

    return image;
}

PNG_Image loadPNG(FILE * in)
{
    PNG_Image image;
    Chunk chunk;
    DataStream stream;

    image = getEmptyPNG_Image();

    loadSignature(image.signature, in);
    /* TODO: Validate signature. */

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
    freeFixedDataList(chunk.data,1);
}

void writePNG(PNG_Image image, FILE * out)
{
    writeSignature(image.signature, out);
    writeHeader(image.header, out);

    writeRenderingIntent(image.renderingIntent, out);
    writeImageGamma(image.imageGamma, out);
    writePixelDimensions(image.pixelDimensions, out);
    writeTimeStamp(image.timeStamp, out);
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
    printFixedDataList(chunk.data, printByte);

    fread(&chunk.CRC, sizeof(INT32), 1, in);
    chunk.CRC = htonl(chunk.CRC);
    verbosePrint("Chunk CRC: %ld\n", chunk.CRC);

    validateCRC(chunk);

    return chunk;
}

FixedDataList readBytes(size_t count, FILE * in)
{
    FixedDataList list;
    size_t i;
    BYTE * b;

    list = getNewFixedDataList(sizeof(void *), count);

    for(i = 0; i < count; ++i){
        b = malloc(sizeof(BYTE));
        *b = getc(in);
        list.list[i] = b;
    }

    return list;
}

void validateCRC(Chunk chunk)
{
    FixedDataList checkData;
    size_t i;
    INT32 calcCRC;

    checkData = getNewFixedDataList(sizeof(void *),chunk.length + 4);

    for(i = 0; i < 4; ++i)
        checkData.list[i] = &chunk.type[i];

    for(i = i; i < (chunk.length + 4); ++i)
        checkData.list[i] = chunk.data.list[i-4];


    calcCRC = crc32(checkData);

    if(calcCRC != chunk.CRC){
        printError("Chunk has invalid checksum: chunk %d != calc %d\n", chunk.CRC, calcCRC );
        exit(1);
    } else
        verbosePrint("Chunk has valid checksum!\n");

    /* Figure out how to free this memory. */
/*    freeFixedDataList(checkData, 0);
      for(i = 0; i < 4; ++i){
      free(checkData.list[i]);
      }*/
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

unsigned int crc32(FixedDataList data){

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

    if(header.colorType == GREYSCALE_COLOR){
	backgroundColor->greyscale = read16BitsNumber(&stream);
    } else if(header.colorType == GREYSCALE_ALPHA_COLOR){
	backgroundColor->greyscaleAlpha.greyscale = read16BitsNumber(&stream);
	backgroundColor->greyscaleAlpha.alpha = max;
    }
/* convert -size 1x1 xc:transparent -fill 'rgba(180, 180, 180, 0.8)' -draw 'rectangle 0,0 1,1' bgbox.png
*/
/*    read16BitsNumber(&stream);
    readStreamByte(&stream); */

    return backgroundColor;
}

INT32 getMaximumValue(ImageHeader header)
{
    return pow(header.bitDepth,8) - 1;
}
