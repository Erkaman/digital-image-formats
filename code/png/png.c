#include "png.h"

void dumpPNG(FILE * in, FILE * out)
{
    PNG_Image image;

    image = loadPNG(in);
    writePNG(image,out);
}

PNG_Signature loadPNG_Signature(FILE * in)
{
    PNG_Signature signature;

    signature.signatureData = allocateDataContainer(8);

    fread(signature.signatureData.data, sizeof(BYTE), 8, in);
    signature.signatureData.size = 8;

    return signature;
}

PNG_Image loadPNG(FILE * in)
{
    PNG_Image image;

    image.signature = loadPNG_Signature(in);
    /* TODO: Validate signature. */

    image.header = loadPNG_ImageHeader(in);

    return image;
}

void writePNG(PNG_Image image, FILE * out)
{
    writePNG_Signature(image.signature, out);
}

void writePNG_Signature(PNG_Signature signature, FILE * out)
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

PNG_Chunk loadPNG_Chunk(FILE * in)
{
    PNG_Chunk chunk;

    verbosePrint("Loading chunk...\n");

    fread(&chunk.length, sizeof(uint32_t), 1, in);
    chunk.length = htonl(chunk.length);

    verbosePrint("Chunk length: %ld\n", chunk.length);

    fread(chunk.chunkType, sizeof(char), 4, in);
    chunk.chunkType[4] = '\0';
    verbosePrint("Chunk Type: %s\n", chunk.chunkType);

    fread(chunk.chunkData.data, sizeof(BYTE), chunk.length, in);
    signature.signatureData.size = chunk.length;

    return chunk;
}

PNG_ImageHeader loadPNG_ImageHeader(FILE * in)
{
    PNG_ImageHeader header;
    PNG_Chunk headerChunk;

    headerChunk = loadPNG_Chunk(in);

    headerChunk = headerChunk;
    header = header;
    in = in;

    return header;
}
