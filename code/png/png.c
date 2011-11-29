#include "png.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "print_funcs.h"

void dumpPNG(FILE * in, FILE * out)
{
    PNG_Image image;

    image = loadPNG(in);
    verbosePrint("lol\n");
    writePNG(image,out);
}

void loadSignature(BYTE * signature, FILE * in)
{
    fread(signature, sizeof(BYTE),SIGNATURE_LENGTH , in);
}

PNG_Image loadPNG(FILE * in)
{
    PNG_Image image;
    Chunk chunk;

    loadSignature(image.signature, in);
    /* TODO: Validate signature. */

    image.header = loadImageHeader(in);

    while(1){

        chunk = loadChunk(in);

        if(!strcmp(chunk.chunkType, IEND))
            break;
        else if(!strcmp(chunk.chunkType, IDAT)){
            /* read the rest of the IDATs */
        }else {
            if(!isCriticalChunk(chunk)){
                printWarning("Unknown ancillary chunk %s found, skipping chunk.",
                             chunk.chunkType);
            } else{
                printError("Unknown critcal chunk %s found.", chunk.chunkType);
                exit(1);
            }
        }
    }

    return image;
}

void writePNG(PNG_Image image, FILE * out)
{
    writeSignature(image.signature, out);
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

    fread(chunk.chunkType, sizeof(char), 4, in);
    chunk.chunkType[4] = '\0';
    verbosePrint("Chunk Type: %s\n", chunk.chunkType);

    /* TODO: Remember to free this memory! */
    chunk.chunkData = getNewFixedDataList(sizeof(BYTE), chunk.length);
    fread(chunk.chunkData.list, sizeof(BYTE), chunk.length, in);

    fread(&chunk.CRC, sizeof(INT32), 1, in);
    chunk.CRC = htonl(chunk.CRC);
    verbosePrint("Chunk CRC: %ld\n", chunk.CRC);

    validateCRC(chunk);

    return chunk;
}

void validateCRC(Chunk chunk)
{
    FixedDataList checkData;
    size_t i;
    INT32 calcCrc32;

    /* Get the data to be validated: the chunk type and the chunk data */

    /* The chunk length bytes are included in the calculation. */
    checkData = getNewFixedDataList(sizeof(BYTE),chunk.length + 4);

    for(i = 0; i < 4; ++i)
        checkData.list[i] = &chunk.chunkType[i];

    for(i = i; i < (chunk.length + 4); ++i)
        checkData.list[i] = &chunk.chunkData.list[i-4];

    verbosePrint("CRC data:\n");
    printFixedDataList(checkData, printByte);
    verbosePrint("wut:\n");

    calcCrc32 = crc32(checkData);
}

int isCriticalChunk(Chunk chunk)
{
    return isupper((BYTE)chunk.chunkType[0]);
}

ImageHeader loadImageHeader(FILE * in)
{
    ImageHeader header;
    Chunk headerChunk;

    headerChunk = loadChunk(in);

    headerChunk = headerChunk;
    header = header;
    in = in;

    return header;
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
