#ifndef _DEFLATE_H_
#define _DEFLATE_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include "../common.h"
#include "deflate.h"
#include "main.h"

DataContainer deflateDecompress(DataContainer data);

#define HUFFMAN_CODES 288
#define DISTANCE_CODES 30
#define LENGTH_CODES 29

typedef struct{
    BYTE BFINAL; /* Is this the last data block? */
    BYTE BTYPE; /* Data compression type. */
} DEFLATE_BlockHeader;

typedef struct{
    /* the code value of the code*/
    unsigned short codeValue;
    unsigned short litteralValue;

    /* maxvalue: 256 */
    /* The codes whose lengths are zero are ignored.  */
    int codeLength;
} HuffmanCode;

typedef struct {
    HuffmanCode codes[HUFFMAN_CODES];
    unsigned short size;
} CodesList;

typedef struct{
    DataContainer stream;
    unsigned long position;
} DataStream;

typedef struct{
    unsigned short extraBits;
    unsigned short minDist;
} DistanceTableEntry;

typedef struct{
    unsigned short extraBits;
    unsigned short minLength;
} LengthTableEntry;

#define DATA_STREAM_GROW_FACTOR 2

void addElementToStream(DataStream * stream, BYTE element);

void setFixedHuffmanCodes(void);
void setFixedDistanceCodes(void);

void printDEFLATE_BlockHeader(DEFLATE_BlockHeader header);

HuffmanCode readCode(CodesList codes,  DataStream * stream);

unsigned short appendBit(unsigned short codeVale, DataStream * stream);

#define BTYPE_NO_COMPRESSION 0
#define BTYPE_COMPRESSED_FIXED_HUFFMAN_CODES 1
#define BTYPE_COMPRESSED_DYNAMIC_HUFFMAN_CODES 2
#define BTYPE_RESERVED 3


#define END_OF_BLOCK 256
#define LITTERAL_VALUES_MAX 255
#define DISTANCE_MIN 257
#define DISTANCE_MAX 285

unsigned short readRestOfLengthCode(
    unsigned short code,
    DataStream * compressedStream);

unsigned short readRestOfDistanceCode(unsigned short code,DataStream * compressedStream);



BYTE getNextByte(DataStream * stream);

void printCode(HuffmanCode code);
int getBitToggled(unsigned short value,int bit);

int getMinimumCodeLength(CodesList codes);

/* return the index of the code with the value, and return -1 if
   it can't be found. */
int findCode(CodesList codes,unsigned short codeValue, unsigned short codeLength);

/* Input the codes in LSB order by in which the lower bits actually are the highest bits! */

unsigned int inputCodeLSBRev(int codeSize, DataStream * stream);

unsigned int inputCodeLSB(int codeSize, DataStream * stream);

void readNonCompresedBlock(DataStream * compressedStream, DataStream * decompressedStream);

void readCompresedBlock(
    CodesList huffmanCodes,
    CodesList distanceCodes,
    DataStream * compressedStream,
    DataStream * decompressedStream);

DEFLATE_BlockHeader readDEFLATE_BlockHeader(DataStream * stream);


void decodeLengthDistancePair(
    HuffmanCode lengthCode,
    CodesList distanceCodes,
    DataStream * compressedStream,
    DataStream * decompressedStream);

void outputLengthDistancePair(
    unsigned short lengthCode,
    unsigned short distanceCode,
    DataStream * decompressedStream);

#endif /* _DEFLATE_H_ */
