#ifndef _PNG_H_
#define _PNG_H_

#include "../common.h"
#include <stdint.h>

typedef struct {
    DataContainer signatureData;
} PNG_Signature;


typedef struct {
    uint32_t length;
    char chunkType[5];
    DataContainer chunkData;
    uint32_t CRC;
} PNG_Chunk;

typedef struct {
    uint32_t width;
    uint32_t height;

    BYTE bitDepth;
    BYTE colorType;

    BYTE compressionMethod;
    BYTE filterMethod;
    BYTE interlaceMethod;

} PNG_ImageHeader;

typedef struct {
    PNG_Signature signature;

    PNG_ImageHeader header;

} PNG_Image;


void dumpPNG(FILE * in, FILE * out);

PNG_Image loadPNG(FILE * in);
PNG_Signature loadPNG_Signature(FILE * in);
PNG_ImageHeader loadPNG_ImageHeader(FILE * in);
PNG_Chunk loadPNG_Chunk(FILE * in);

void writePNG(PNG_Image image, FILE * out);

void writePNG_Signature(PNG_Signature signature, FILE * out);


#endif /* _PNG_H_ */

