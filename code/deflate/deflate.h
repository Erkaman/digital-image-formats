#ifndef _DEFLATE_H_
#define _DEFLATE_H_

#include "../common.h"

DataContainer deflateDecompress(DataContainer data);

unsigned int inputCodeLSB(int codeSize);

typedef struct{
    BYTE BFINAL; /* Is this the last data block? */
    BYTE BTYPE; /* Data compression type. */
} DEFLATE_BlockHeader;

DEFLATE_BlockHeader readDEFLATE_BlockHeader(void);
void writeDEFLATE_BlockHeader(DEFLATE_BlockHeader header);

BYTE getNextByte(void);

#define BTYPE_NO_COMPRESSION 0
#define BTYPE_COMPRESSED_FIXED_HUFFMAN_CODES 1
#define BTYPE_COMPRESSED_DYNAMIC_HUFFMAN_CODES 2
#define BTYPE_RESERVED 3

#endif /* _DEFLATE_H_ */
