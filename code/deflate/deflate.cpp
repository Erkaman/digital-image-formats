
#include "../io.h"
#include "../bits.h"
#include "deflate.h"
#include "../data_stream.h"
#include "../huffman/huffman.h"

#include <cstdlib>

using std::vector;

#define HUFFMAN_CODES 288
#define DISTANCE_CODES 30
#define LENGTH_CODES 29

typedef struct{
    BYTE BFINAL; /* Is this the last data block? */
    BYTE BTYPE; /* Data compression type. */
} DEFLATE_BlockHeader;

typedef struct{
    unsigned short extraBits;
    unsigned short minDist;
} DistanceTableEntry;

typedef struct{
    unsigned short extraBits;
    unsigned short minLength;
} LengthTableEntry;

typedef struct {
    unsigned short HLIT; /* 5 bits. number of literal/length codes - 257(257-286).   */
    unsigned short HDIST; /* 5 bits. Number of distance codes - 1 (1 - 32)  */
    unsigned short HCLEN; /* 4 bits. Number of code length codes - 4 (4 - 19) */
} DEFLATE_DynamicBlockHeader;

RevCodesList loadLiteralLengthCodes(
    unsigned short HLIT,
    const RevCodesList & codeLengthCodes,
    BitReader * compressedStream);

RevCodesList loadUsingCodeLengthCodes(
    unsigned short length,
    unsigned short alphabetLength,
    const RevCodesList & codeLengthCodes,
    BitReader * compressedStream);

RevCodesList loadDistanceCodes(
    unsigned short HDIST,
    const RevCodesList & codeLengthCodes,
    BitReader * compressedStream);

void loadDynamicTables(
    RevCodesList * huffmanCodes,
    RevCodesList * distanceCodes,
    BitReader * compressedStream);

DEFLATE_DynamicBlockHeader loadDEFLATE_DynamicBlockHeader(BitReader * compressedStream);

void printDEFLATE_DynamicBlockHeader(DEFLATE_DynamicBlockHeader blockHeader);

#define DATA_STREAM_GROW_FACTOR 2


void setFixedHuffmanCodes(void);
void setFixedDistanceCodes(void);

void printDEFLATE_BlockHeader(DEFLATE_BlockHeader header);

#define BTYPE_NO_COMPRESSION 0
#define BTYPE_COMPRESSED_FIXED_HUFFMAN_CODES 1
#define BTYPE_COMPRESSED_DYNAMIC_HUFFMAN_CODES 2
#define BTYPE_RESERVED 3

#define END_OF_BLOCK 256
#define LITTERAL_VALUES_MAX 255
#define DISTANCE_MIN 257
#define DISTANCE_MAX 285

unsigned short readRestOfLengthCode(
    unsigned int code,
    BitReader * compressedStream);

unsigned int readRestOfDistanceCode(unsigned int code,BitReader * compressedStream);

void readNonCompresedBlock(BitReader & compressedStream, vector<BYTE> & decompressedList);

void readCompresedBlock(
    const RevCodesList & huffmanCodes,
    const RevCodesList & distanceCodes,
    BitReader * compressedStream,
    vector<BYTE> & decompressedList);

DEFLATE_BlockHeader readDEFLATE_BlockHeader(BitReader & stream);

void decodeLengthDistancePair(
    unsigned int lengthCode,
    const RevCodesList & distanceCodes,
    BitReader * compressedStream,
    vector<BYTE> & decompressedList);

void outputLengthDistancePair(
    unsigned short lengthCode,
    unsigned short distanceCode,
    vector<BYTE> &  decompressedList);

/* Globals */

int remainingPacketBits;

RevCodesList fixedHuffmanCodes;
RevCodesList fixedDistanceCodes;

LengthTableEntry lengthTable[LENGTH_CODES] = {

    {0,3},
    {0,4},
    {0,5},
    {0,6},
    {0,7},
    {0,8},
    {0,9},
    {0,10},

    {1,11},
    {1,13},
    {1,15},
    {1,17},

    {2,19},
    {2,23},
    {2,27},
    {2,31},

    {3,35},
    {3,43},
    {3,51},
    {3,59},

    {4,67},
    {4,83},
    {4,99},
    {4,115},

    {5,131},
    {5,163},
    {5,195},
    {5,227},

    {0, 258}
};

DistanceTableEntry distanceTable[DISTANCE_CODES] = {
    {0,1},
    {0,2},
    {0,3},
    {0,4},

    {1,5},
    {1,7},

    {2,9},
    {2,13},

    {3,17},
    {3,25},

    {4,33},
    {4,49},

    {5,65},
    {5,97},

    {6,129},
    {6,193},

    {7,257},
    {7,385},

    {8,513},
    {8,769},

    {9,1025},
    {9,1537},

    {10,2049},
    {10,3073},

    {11,4097},
    {11,6145},

    {12,8193},
    {12,12289},

    {13,16385},
    {13,24577},
};

void setFixedHuffmanCodes(void)
{
    int i;
    HuffmanCode code;

    /* Litteral value 256 is the end code.*/

    /* Literal values 0-143 are given the codes
       00110000(=48=0x30) - 10111111(191=0xbf) of length 8*/
    for(i = 0; i <= 143; ++i){
        code.value = 0x30 + i;
        code.length = 8;
        fixedHuffmanCodes[code] = i;
    }

    /* Literal values 144-255 are given the codes
       1 1001 0000(=400=0x190) - 1 1111 1111(=511=0x1ff) of length 9*/
    for(i = 144; i <= 255; ++i){
        code.value = 0x190 + i - 144;
        code.length = 9;
        fixedHuffmanCodes[code] = i;
    }

    /* Literal values 256-279 are given the codes
       000 0000(=0=0x0) - 001 0111(=23=0x17) of length 7*/
    for(i = 256; i <= 279; ++i){
        code.value = 0x0 + i - 256;
        code.length = 7;
        fixedHuffmanCodes[code] = i;
    }

    /* Literal values 280-287 are given the codes
       1100 0000(=192=0xc0) - 1100 0111(=199=0xc7) of length 8*/
    for(i = 280; i <= 287; ++i){
        code.value = 0xc0 + i - 280;
        code.length = 8;
        fixedHuffmanCodes[code] = i;
    }
}

void setFixedDistanceCodes(void)
{
    int i;
    HuffmanCode code;

    /*
      "Distance codes 0-31 are represented by (fixed-length) 5-bit codes"
    */
    for(i = 0; i <= 31; ++i){
        code.value = i;
        code.length = 5;
        fixedDistanceCodes[code] = i;
    }
}

void readNonCompresedBlock(BitReader & compressedStream, vector<BYTE> & decompressedList)
{
    BYTE b1;
    BYTE b2;
    unsigned long uncompressedBlockSize;
/* if dynaic codes are used a new table is read in before the decompression
   of the data. */

    compressedStream.nextByte();
    /* FIXME: does this handle big and little endian? */
    /* Implement a routine for doing this. */
    b1 = compressedStream.readBits(8);
    b2 = compressedStream.readBits(8);

    uncompressedBlockSize = (b1 + b2 * 256);

    /* Skip NLEN */
    compressedStream.readBits(8);
    compressedStream.readBits(8);

    for(unsigned long i = 0; i < uncompressedBlockSize; ++i){

        BYTE b = compressedStream.readBits(8);
        decompressedList.push_back(b);
    }
}

vector<BYTE> deflateDecompress(vector<BYTE> data)
{
    DEFLATE_BlockHeader header;
    vector<BYTE> decompressedList;

    BitIterReader inBits(data.begin(), LSBF);

/*    translateCodesTest(); */

    /* TODO: ???? correct endianess?  */
/*    compressedStream = getNewDataStream(data, ENDIAN_BIG); */

    /* the first two bytes of the compressed stream were headers bytes, which
       we have already read, so skip them. */
    inBits.readBits(8);
    inBits.readBits(8);

    /* Put this into a preparatory function? */
    setFixedHuffmanCodes();
    setFixedDistanceCodes();

    remainingPacketBits = 8;

    do{
        /* Skip past to the next byte in the stream where the next block begins. */

        header = readDEFLATE_BlockHeader(inBits);
        printDEFLATE_BlockHeader(header);

        if(header.BTYPE == BTYPE_NO_COMPRESSION){

            readNonCompresedBlock(inBits, decompressedList);

        } else{

            RevCodesList huffmanCodes;
            RevCodesList distanceCodes;

            if (header.BTYPE == BTYPE_COMPRESSED_DYNAMIC_HUFFMAN_CODES) {

                loadDynamicTables(&huffmanCodes, &distanceCodes, &inBits);

            } else if(header.BTYPE == BTYPE_COMPRESSED_FIXED_HUFFMAN_CODES) {
                huffmanCodes = fixedHuffmanCodes;
                distanceCodes = fixedDistanceCodes;
            }

            readCompresedBlock(
                huffmanCodes,
                distanceCodes,
                &inBits,
                decompressedList);
        }
    }while(header.BFINAL == 0);

    return decompressedList;
}



void loadDynamicTables(
    RevCodesList * huffmanCodes,
    RevCodesList * distanceCodes,
    BitReader * compressedStream)
{
    DEFLATE_DynamicBlockHeader blockHeader;

    RevCodesList codeLengthCodes;

    blockHeader = loadDEFLATE_DynamicBlockHeader(compressedStream);
    printDEFLATE_DynamicBlockHeader(blockHeader);

    codeLengthCodes = loadCodeLengthCodes(blockHeader.HCLEN, compressedStream);

    *huffmanCodes = loadLiteralLengthCodes(
        blockHeader.HLIT,
        codeLengthCodes,
        compressedStream);

    *distanceCodes = loadDistanceCodes(
        blockHeader.HDIST,
        codeLengthCodes,
        compressedStream);
}

RevCodesList loadLiteralLengthCodes(
    unsigned short HLIT,
    const RevCodesList & codeLengthCodes,
    BitReader * compressedStream)
{
    return loadUsingCodeLengthCodes(
        HLIT + 257,
        HUFFMAN_CODES,
        codeLengthCodes,
        compressedStream);
}


RevCodesList loadDistanceCodes(
    unsigned short HDIST,
    const RevCodesList & codeLengthCodes,
    BitReader * compressedStream)
{
    return loadUsingCodeLengthCodes(
        HDIST + 1,
        DISTANCE_CODES,
        codeLengthCodes,
        compressedStream);
}

RevCodesList loadUsingCodeLengthCodes(
    unsigned short length,
    unsigned short alphabetLength,
    const RevCodesList & codeLengthCodes,
    BitReader * compressedStream)
{
    size_t i;

    CodeLengths codeLengths(alphabetLength);
    unsigned int translatedCode;
    int codesLenI;

    codesLenI = 0;

    fill(codeLengths.begin(), codeLengths.end(), 0);

    while(1){
        translatedCode = readCode(codeLengthCodes,compressedStream);

        if(/*code.litteralValue >= 0 &&*/ translatedCode <= 15){

            codeLengths[codesLenI++] = translatedCode;

        } else if(translatedCode >= 16){

	    vector<BYTE> repeated;

	    /* TODO: does this work????*/
            if(translatedCode == 16){
		repeated =
		    repeatPreviousLengthCode(
			codeLengths[codesLenI-1],
			compressedStream);
            }else if(translatedCode == 17){
		repeated = repeatZeroLengthCode(3,3, compressedStream);
            } else if(translatedCode == 18){
		repeated = repeatZeroLengthCode(11,7, compressedStream);
            }

	    for(i = 0; i < repeated.size(); ++i){
		codeLengths[codesLenI++] = repeated[i];
	    }

        }

        if(codesLenI == length){
            break;
        }
    }

    return reverseCodesList(translateCodes(codeLengths));
}

DEFLATE_DynamicBlockHeader loadDEFLATE_DynamicBlockHeader(BitReader * compressedStream)
{
    DEFLATE_DynamicBlockHeader blockHeader;

    blockHeader.HLIT = compressedStream->readBits(5);
    blockHeader.HDIST = compressedStream->readBits(5);
    blockHeader.HCLEN = compressedStream->readBits(4);

    return blockHeader;
}

void printDEFLATE_DynamicBlockHeader(DEFLATE_DynamicBlockHeader blockHeader)
{
    verbosePrint("DEFLATE Dynamic Block Header\n");

    verbosePrint("HLIT:%d\n", blockHeader.HLIT);
    verbosePrint("HDIST:%d\n", blockHeader.HDIST);
    verbosePrint("HCLEN:%d\n", blockHeader.HCLEN);
}

void readCompresedBlock(
    const RevCodesList & huffmanCodes,
    const RevCodesList & distanceCodes,
    BitReader * compressedStream,
    vector<BYTE> & decompressedList)
{
    size_t code;

    while(1){

        code = readCode(huffmanCodes, compressedStream);

        /* If the code is a simple literal value. */
        if(code <= LITTERAL_VALUES_MAX){
            /* */
            decompressedList.push_back((BYTE)code);
        }
        else if(code == END_OF_BLOCK){
            /* If the code is a end of block code, then stop.*/
            break;
        } else if(code >= DISTANCE_MIN &&
                  code <= DISTANCE_MAX){

            decodeLengthDistancePair(
                code,
                distanceCodes,
                compressedStream,
                decompressedList);
        } else{
            printError("Invalid code found:\n");
            printf("%ld\n", code);
        }
    }
}

void decodeLengthDistancePair(
    unsigned int lengthCode,
    const RevCodesList & distanceCodes,
    BitReader * compressedStream,
    vector<BYTE> & decompressedList)
{
    unsigned short fullLengthCode;

    unsigned short fullDistanceCode;

    unsigned int distanceCode;

    fullLengthCode =  readRestOfLengthCode(
        lengthCode,
        compressedStream);

    /* Read the following distance code. */
    distanceCode = readCode(distanceCodes, compressedStream);

    fullDistanceCode = readRestOfDistanceCode(distanceCode,compressedStream);

    outputLengthDistancePair(fullLengthCode, fullDistanceCode, decompressedList);
}

void outputLengthDistancePair(
    unsigned short lengthCode,
    unsigned short distanceCode,
    vector<BYTE> &  decompressedList)
{
    unsigned short i;
    BYTE toAdd;

    for(i = 0; i < lengthCode; ++i){

        toAdd = decompressedList[decompressedList.size() - distanceCode];

        decompressedList.push_back(toAdd);
    }
}

unsigned int readRestOfDistanceCode(unsigned int code,BitReader * compressedStream)
{
    unsigned short distanceCode;
    DistanceTableEntry entry;

    entry = distanceTable[code];

    if(entry.extraBits != 0){
        distanceCode = entry.minDist + compressedStream->readBits(entry.extraBits);
    } else{
        distanceCode = entry.minDist;
    }

    return distanceCode;
}


unsigned short readRestOfLengthCode(
    unsigned int code,
    BitReader * compressedStream)
{
    unsigned short lengthCode;
    LengthTableEntry entry;

    entry = lengthTable[code - 257];

    if(entry.extraBits != 0)
        lengthCode = entry.minLength + compressedStream->readBits(entry.extraBits);
    else
        lengthCode = entry.minLength;

    return lengthCode;
}

DEFLATE_BlockHeader readDEFLATE_BlockHeader(BitReader & stream)
{
    DEFLATE_BlockHeader header;

    header.BFINAL = stream.readBits(1);
    header.BTYPE = stream.readBits(2);

    return header;
}

void printDEFLATE_BlockHeader(DEFLATE_BlockHeader header)
{
    verbosePrint("DEFLATE Header Block\n");
    verbosePrint("BFINAL:%d\n",header.BFINAL);

    verbosePrint("BTYPE(Block Type): %d(",header.BTYPE);

    switch(header.BTYPE){
    case BTYPE_NO_COMPRESSION:
        verbosePrint("No Compression");
        break;
    case BTYPE_COMPRESSED_FIXED_HUFFMAN_CODES:
        verbosePrint("Compressed with fixed Huffman codes");
        break;
    case  BTYPE_COMPRESSED_DYNAMIC_HUFFMAN_CODES:
        verbosePrint("Compressed with dynamic Huffman codes");
        break;
    case BTYPE_RESERVED:
        printError("Reserved(error");
        printf("hai\n");
        exit(0);
        break;
    }
    verbosePrint(")\n");
    verbosePrint("\n");

    verbosePrint("\n");
}
