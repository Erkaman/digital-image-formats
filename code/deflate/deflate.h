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

unsigned int inputCodeLSB(int codeSize);

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

void setFixedHuffmanCodes(void);

DEFLATE_BlockHeader readDEFLATE_BlockHeader(void);
void printDEFLATE_BlockHeader(DEFLATE_BlockHeader header);

BYTE getNextByte(void);

/*void readFixedHuffmanCodes */

#define BTYPE_NO_COMPRESSION 0
#define BTYPE_COMPRESSED_FIXED_HUFFMAN_CODES 1
#define BTYPE_COMPRESSED_DYNAMIC_HUFFMAN_CODES 2
#define BTYPE_RESERVED 3

#define TREE_SIZE 288

void printCode(HuffmanCode code);
int getBitToggled(unsigned short value,int bit);

int getMinimumCodeLength(HuffmanCode * codes);

/* return the index of the code with the value, and return -1 if
 it can't be found. */
int findCode(HuffmanCode * codes, unsigned short value);

unsigned int inputCodeLSB(int codeSize);

/* Input the codes in LSB order by in which the lower bits actually are the highest bits! */
unsigned int inputCodeLSBRev(int codeSize);

#endif /* _DEFLATE_H_ */
