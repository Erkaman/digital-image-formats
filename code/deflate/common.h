#ifndef _COMMON_H_
#define _COMMON_H_

#include "../huffman/huffman.h"

#define HUFFMAN_CODES 288
#define DISTANCE_CODES 30
#define LENGTH_CODES 29

typedef struct{
    BYTE BFINAL; /* Is this the last data block? */
    BYTE BTYPE; /* Data compression type. */
} DEFLATE_BlockHeader;

#define BTYPE_NO_COMPRESSION 0
#define BTYPE_COMPRESSED_FIXED_HUFFMAN_CODES 1
#define BTYPE_COMPRESSED_DYNAMIC_HUFFMAN_CODES 2
#define BTYPE_RESERVED 3

#define END_OF_BLOCK 256
#define LITTERAL_VALUES_MAX 255
#define LENGTH_MIN 257
#define LENGTH_MAX 285

typedef struct{
    unsigned short extraBits;
    unsigned short minDist;
} DistanceTableEntry;

typedef struct{
    unsigned short extraBits;
    unsigned short minLength;
} LengthTableEntry;

extern const LengthTableEntry lengthTable[LENGTH_CODES];

extern const DistanceTableEntry distanceTable[DISTANCE_CODES];

CodesList getFixedHuffmanCodes(void);
CodesList getFixedDistanceCodes(void);

#endif /* _COMMON_H_ */
