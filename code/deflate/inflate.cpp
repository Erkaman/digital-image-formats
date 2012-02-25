#include "inflate.h"
#include "../io.h"
#include "../bits.h"
#include "../data_stream.h"
#include "../huffman/huffman.h"
#include "../lzss/lzss_util.h"
#include "common.h"

#include <cstdlib>

using std::vector;

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

void printDEFLATE_BlockHeader(DEFLATE_BlockHeader header);


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

void readNonCompresedBlock(BitReader & compressedStream, vector<BYTE> & decompressedList)
{
    BYTE b1;
    BYTE b2;
    unsigned long uncompressedBlockSize;
/* if dynaic codes are used a new table is read in before the decompression
   of the data. */

    compressedStream.readNextByte();
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

vector<BYTE> inflate(vector<BYTE> data)
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
    RevCodesList fixedHuffmanCodes  = reverseCodesList(getFixedHuffmanCodes());
    RevCodesList fixedDistanceCodes  = reverseCodesList(getFixedDistanceCodes());

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
        } else if(code >= LENGTH_MIN &&
                  code <= LENGTH_MAX){

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

    Token token;
    token.length = fullLengthCode;
    token.offset = fullDistanceCode;

    decodeToken(token, decompressedList);
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
        exit(0);
        break;
    }
    verbosePrint(")\n");
    verbosePrint("\n");

    verbosePrint("\n");
}
