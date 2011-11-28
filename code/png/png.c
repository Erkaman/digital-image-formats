#include "png.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

void dumpPNG(FILE * in, FILE * out)
{
    PNG_Image image;

    image = loadPNG(in);
    writePNG(image,out);
}

Signature loadSignature(FILE * in)
{
    Signature signature;

    signature.signatureData = allocateDataContainer(8);

    fread(signature.signatureData.data, sizeof(BYTE), 8, in);
    signature.signatureData.size = 8;

    return signature;
}

PNG_Image loadPNG(FILE * in)
{
    PNG_Image image;
    Chunk chunk;

    image.signature = loadSignature(in);
    /* TODO: Validate signature. */

    image.header = loadImageHeader(in);

    while(1){

	chunk = loadChunk(in);

	if(!strcmp(chunk.chunkType, IEND)){
	    break;
	} else if(!strcmp(chunk.chunkType, IDAT)){
	    /* read the rest of the IDATs */
	} else {
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

void writeSignature(Signature signature, FILE * out)
{
    unsigned long i;

    fprintf(out, "PNG Signature:\n");
    for(i = 0; i < signature.signatureData.size; ++i){
        if(i != (signature.signatureData.size - 1)){
            fprintf(out, "%d ", signature.signatureData.data[i]);
        } else{
            fprintf(out, "%d\n", signature.signatureData.data[i]);
        }
    }
}

Chunk loadChunk(FILE * in)
{
    Chunk chunk;

    verbosePrint("Loading chunk...\n");

    fread(&chunk.length, sizeof(uint32_t), 1, in);
    chunk.length = htonl(chunk.length);

    verbosePrint("Chunk length: % ld\n", chunk.length);

    fread(chunk.chunkType, sizeof(char), 4, in);
    chunk.chunkType[4] = '\0';
    verbosePrint("Chunk Type: %s\n", chunk.chunkType);

    /* TODO: Remember to free this memory! */
    chunk.chunkData = allocateDataContainer(chunk.length);
    fread(chunk.chunkData.data, sizeof(BYTE), chunk.length, in);

    fread(&chunk.CRC, sizeof(uint32_t), 1, in);
    chunk.CRC = htonl(chunk.CRC);
    verbosePrint("Chunk CRC: %ld\n", chunk.CRC);

    validateCRC(chunk);

    /* Validate CRC. */

    return chunk;
}

void validateCRC(Chunk chunk)
{
    DataContainer checkData;
    unsigned long i;
    unsigned long calcCrc32;

    /* Get the data to be validated: the chunk type and the chunk data */

    /* The chunk length bytes are included in the calculation. */
    checkData = allocateDataContainer(chunk.length + 4);

    for(i = 0; i < 4; ++i)
	checkData.data[i] = chunk.chunkType[i];

    for(i = i; i < (chunk.length + 4); ++i)
	checkData.data[i] = chunk.chunkData.data[i-4];

    verbosePrint("CRC data:\n");
    printData(checkData);

    calcCrc32 = crc32(checkData);

/*    verbosePrint("crc32:%d\n",); */

    /* */
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

unsigned int crc32(DataContainer data){

    unsigned long reminder = 0xFFFFFFFF; /* standard initial value in CRC32 */
    unsigned long i;
    unsigned long bit;

    for(i = 0; i < data.size; ++i){
        reminder ^= data.data[i]; /* must be zero extended */
        for(bit = 0; bit < 8; bit++)
            if(reminder & 0x01)
                reminder = (reminder >> 1) ^ CRC32_POLY;
            else
                reminder >>= 1;
    }

    return reminder ^ 0xFFFFFFFF;
}
