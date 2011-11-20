#ifndef _RLE_H_
#define _RLE_H_

#include "../common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

void RLE_Encode(FILE * in, FILE * out);
void RLE_Decode(FILE * in, FILE * out);

void packBitsEncode(FILE * in, FILE * out);
void packBitsDecode(FILE * in, FILE * out);

void writeRawPacket(BYTE length,BYTE * data,FILE * out);
void writeRunLengthPacket(BYTE length,BYTE data,FILE * out);

void printHelp(void);

#define RUN_LENGTH_PACKET 1
#define RAW_PACKET 0

#endif /* _RLE_H_ */
