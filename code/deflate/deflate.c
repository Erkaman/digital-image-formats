#include "deflate.h"
#include "../common.h"
#include <stdlib.h>
#include <ctype.h>

int remainingPacketBits;

CodesList fixedHuffmanCodes;
CodesList fixedDistanceCodes;

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

int getMinimumCodeLength(CodesList codes)
{
    int min;
    int i;

    /* find the first non-zero length code */
    for(i = 0; i < codes.size; ++i){
        if(codes.codes[i].codeLength != 0){
            min = codes.codes[i].codeLength;
            break;
        }
    }

    /* find the minimum length code. */
    for(; i < codes.size; ++i)
        if(codes.codes[i].codeLength != 0 && codes.codes[i].codeLength < min)
            min = codes.codes[i].codeLength;

    return min;
}

BYTE getNextByte(DataStream * stream)
{
    return stream->stream.data[++stream->position];
}

void setFixedHuffmanCodes(void)
{
    int i;
    int codesI = 0;
    HuffmanCode code;

    fixedHuffmanCodes.size = HUFFMAN_CODES;

    verbosePrint("Assigning static huffman codes:\n");

    /* Litteral value 256 is the end code.*/

    /* Literal values 0-143 are given the codes
       00110000(=48=0x30) - 10111111(191=0xbf) of length 8*/
    for(i = 0; i <= 143; ++i){
        code.codeValue = 0x30 + i;
        code.litteralValue = i;
        code.codeLength = 8;
        fixedHuffmanCodes.codes[codesI++] = code;

    }

    /* Literal values 144-255 are given the codes
       1 1001 0000(=400=0x190) - 1 1111 1111(=511=0x1ff) of length 9*/
    for(i = 144; i <= 255; ++i){
        code.codeValue = 0x190 + i - 144;
        code.litteralValue = i;
        code.codeLength = 9;
        fixedHuffmanCodes.codes[codesI++] = code;

    }

    /* Literal values 256-279 are given the codes
       000 0000(=0=0x0) - 001 0111(=23=0x17) of length 7*/
    for(i = 256; i <= 279; ++i){
        code.codeValue = 0x0 + i - 256;
        code.litteralValue = i;
        code.codeLength = 7;
        fixedHuffmanCodes.codes[codesI++] = code;

    }

    /* Literal values 280-287 are given the codes
       1100 0000(=192=0xc0) - 1100 0111(=199=0xc7) of length 8*/
    for(i = 280; i <= 287; ++i){
        code.codeValue = 0xc0 + i - 280;
        code.litteralValue = i;
        code.codeLength = 8;
        fixedHuffmanCodes.codes[codesI++] = code;

    }
}

void setFixedDistanceCodes(void)
{
    int i;
    int codesI = 0;
    HuffmanCode code;

    fixedDistanceCodes.size = DISTANCE_CODES;

    verbosePrint("Assigning distance codes:\n");

    /*
      "Distance codes 0-31 are represented by (fixed-length) 5-bit codes"
    */
    for(i = 0; i <= 31; ++i){
        code.codeValue = i;
        code.litteralValue = i;
        code.codeLength = 5;
        fixedDistanceCodes.codes[codesI++] = code;
    }
}

void readNonCompresedBlock(DataStream * compressedStream, DataStream * decompressedStream)
{
    BYTE b1;
    BYTE b2;
    unsigned long uncompressedBlockSize;
    unsigned long i;

/* if dynaic codes are used a new table is read in before the decompression
   of the data. */

    remainingPacketBits = 8;

    /* FIXME: does this handle big and little endian? */
    b1 = getNextByte(compressedStream);
    b2 = getNextByte(compressedStream);

    verbosePrint("b1:%d\n",b1);
    verbosePrint("b2:%d\n",b2);

    uncompressedBlockSize = (b1 + b2 * 256);

    /* Skip NLEN */
    getNextByte(compressedStream);
    getNextByte(compressedStream);

    for(i = 0; i < uncompressedBlockSize; ++i){
/*        decompressedStream->stream.data[decompressedStream->position++] =
          getNextByte(compressedStream);*/

        addElementToStream(decompressedStream, getNextByte(compressedStream));

        verbosePrint("%c",decompressedStream->stream.data[decompressedStream->position-1]);
    }
}

void addElementToStream(DataStream * stream, BYTE element)
{
    unsigned long newSize;


    verbosePrint("size test: %d = %d\n",
                 stream->position,
                 stream->stream.size);

    /* accommodate the size of the output stream of needed. */
    if( stream->position == stream->stream.size){

        newSize = DATA_STREAM_GROW_FACTOR *
            ( stream->stream.size > 0 ? stream->stream.size : 1);

        verbosePrint("new decompressed stream size allocated: %d\n",newSize);
        stream->stream =
            accommodateDataContainer(
                stream->stream,
                newSize);
    }

    verbosePrint("Added to decompressed stream:%d=%c\n",element,element);    stream->stream.data[stream->position++] = element;
    verbosePrint("new stream position:%d\n", stream->position);
}


DataContainer deflateDecompress(DataContainer data)
{
    DEFLATE_BlockHeader header;
    DataStream compressedStream;
    DataStream decompressedStream;

    CodesList huffmanCodes;
    CodesList distanceCodes;

/*    translateCodesTest();
      return decompressedStream.stream; */

    compressedStream.stream = data;

    /* Remember, the first to bytes of the compressed stream were headers bytes, and
       we have already read them, and so we skip them. */
    compressedStream.position = 2;

    /* No decompressed data has so far been read. */
    decompressedStream.position = 0;
    decompressedStream.stream.size = 0;

    /* Put this into a preparatory function? */
    setFixedHuffmanCodes();
    setFixedDistanceCodes();

    decompressedStream.stream = getEmptyDataContainer();

    remainingPacketBits = 8;

    do{
        /* Skip past to the next byte in the stream where the next block begins. */

        header = readDEFLATE_BlockHeader(&compressedStream);
        printDEFLATE_BlockHeader(header);

        if(header.BTYPE == BTYPE_NO_COMPRESSION){


            readNonCompresedBlock(&compressedStream, &decompressedStream);

	    /* ?????  */
            if(decompressedStream.position > 2){
                ++compressedStream.position;
            }

        } else{

            if (header.BTYPE == BTYPE_COMPRESSED_DYNAMIC_HUFFMAN_CODES) {

                loadDynamicTables(&huffmanCodes, &distanceCodes, &compressedStream);
/*                break; */

            } else if(header.BTYPE == BTYPE_COMPRESSED_FIXED_HUFFMAN_CODES) {
                huffmanCodes = fixedHuffmanCodes;
                distanceCodes = fixedDistanceCodes;
            }

            verbosePrint("Codes have been constructed\n.");

            readCompresedBlock(
                huffmanCodes,
                distanceCodes,
                &compressedStream,
                &decompressedStream);
        }
    }while(header.BFINAL == 0);

    verbosePrint("final size: %d", decompressedStream.position);
    decompressedStream.stream.size = decompressedStream.position;

    return decompressedStream.stream;
}

void loadDynamicTables(
    CodesList * huffmanCodes,
    CodesList * distanceCodes,
    DataStream * compressedStream)
{
    DEFLATE_DynamicBlockHeader blockHeader;

    CodesList codeLengthCodes;

    blockHeader = loadDEFLATE_DynamicBlockHeader(compressedStream);
    printDEFLATE_DynamicBlockHeader(blockHeader);

    codeLengthCodes = loadCodeLengthCodes(blockHeader.HCLEN, compressedStream);

    verbosePrint("code length codes:\n");
    printCodesList(codeLengthCodes);

    *huffmanCodes = loadLiteralLengthCodes(
        blockHeader.HLIT,
        codeLengthCodes,
        compressedStream);

    verbosePrint("huffman codes:\n");
    printCodesList(*huffmanCodes);

    *distanceCodes = loadDistanceCodes(
        blockHeader.HDIST,
        codeLengthCodes,
        compressedStream);

    verbosePrint("distance codes:\n");
    printCodesList(*distanceCodes);
}

CodesList loadLiteralLengthCodes(
    unsigned short HLIT,
    CodesList codeLengthCodes,
    DataStream * compressedStream)
{
    verbosePrint("Loading Literal Length Codes\n");
    return loadUsingCodeLengthCodes(
        HLIT + 257,
        HUFFMAN_CODES,
        codeLengthCodes,
        compressedStream);
}

CodesList loadDistanceCodes(unsigned short HDIST,
                            CodesList codeLengthCodes,
                            DataStream * compressedStream)
{
    verbosePrint("Loading Distance Codes\n");

    return loadUsingCodeLengthCodes(
        HDIST + 1,
        DISTANCE_CODES,
        codeLengthCodes,
        compressedStream);
}

CodesList loadUsingCodeLengthCodes(
    unsigned short length,
    unsigned short alphabetLength,
    CodesList codeLengthCodes,
    DataStream * compressedStream)
{
    int  i;
    BYTE codeLengths[HUFFMAN_CODES];
    HuffmanCode code;
    int codesLenI;

    codesLenI = 0;

    for(i = 0; i < alphabetLength; ++i){
        codeLengths[i] = 0;
    }

    while(1){
        code = readCode(codeLengthCodes, compressedStream);
        verbosePrint("found code:\n");
        printCode(code);

        if(/*code.litteralValue >= 0 &&*/ code.litteralValue <= 15){

	    verbosePrint("outputting simple code %d at index %d\n",
                     code.litteralValue,
                     codesLenI);

            codeLengths[codesLenI++] = code.litteralValue;

        } else if(code.litteralValue == 16){
            repeatPreviousLengthCode(codeLengths, &codesLenI, compressedStream);

        }else if(code.litteralValue == 17){
            repeatZeroLengthCode(codeLengths, &codesLenI, 3, 3, compressedStream);

        } else if(code.litteralValue == 18){
            repeatZeroLengthCode(codeLengths, &codesLenI, 11, 7, compressedStream);
        }

        if(codesLenI == length){
            break;
        }
    }

    return translateCodes(codeLengths, alphabetLength);
}

void repeatPreviousLengthCode(
    BYTE * codeLengths,
    int * i,
    DataStream * compressedStream)
{
    int repeatLength;
    int previousCode;
    int j;

    repeatLength =  3 + inputCodeLSB(2, compressedStream);

    previousCode = codeLengths[(*i) - 1];

    for(j = 0; j < repeatLength; ++j){
        verbosePrint("outputting repeat code length %d at index %d\n",
                     previousCode,
                     *i);
        codeLengths[*i] = 0;
        (*i)++;
    }
}

void repeatZeroLengthCode(
    BYTE * codeLengths,
    int * i,
    int minCodeLength,
    int extraBits,
    DataStream * compressedStream)
{
    int realLength;
    int j;

    /* Repeat codes are recursive!!!!

     Example:
Codes 8, 16 (+2 bits 11),
16 (+2 bits 10) will expand to
12 code lengths of 8 (1 + 6 + 5)*/

    realLength =  minCodeLength + inputCodeLSB(extraBits,compressedStream);

    for(j = 0; j < realLength; ++j){
        verbosePrint("outputting zero length at index %d\n", *i);

        codeLengths[*i] = 0;
        (*i)++;
    }

}

CodesList loadCodeLengthCodes(unsigned short HCLEN,DataStream * compressedStream)
{
    unsigned short realLength;
    BYTE codeLengthOrder[CODE_LENGTH_CODES] = {
        16, 17, 18,0, 8, 7,
        9, 6, 10, 5, 11, 4,
        12, 3, 13, 2, 14, 1, 15};

    BYTE codeLengths[CODE_LENGTH_CODES];

    int i;

    realLength = HCLEN + 4;

    for(i = 0; i < CODE_LENGTH_CODES; ++i)
        codeLengths[i] = 0;

    for(i = 0; i < realLength; ++i)
        codeLengths[codeLengthOrder[i]] = inputCodeLSB(3, compressedStream);

    return translateCodes(codeLengths , CODE_LENGTH_CODES);
}

DEFLATE_DynamicBlockHeader loadDEFLATE_DynamicBlockHeader(DataStream * compressedStream)
{
    DEFLATE_DynamicBlockHeader blockHeader;

    blockHeader.HLIT = inputCodeLSB(5,compressedStream);
    blockHeader.HDIST = inputCodeLSB(5,compressedStream);
    blockHeader.HCLEN = inputCodeLSB(4,compressedStream);

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
    CodesList huffmanCodes,
    CodesList distanceCodes,
    DataStream * compressedStream,
    DataStream * decompressedStream)
{
    HuffmanCode code;

    while(1){

        code = readCode(huffmanCodes, compressedStream);

        printCode(code);

        /* If the code is a simple literal value. */
        if(code.litteralValue <= LITTERAL_VALUES_MAX){
            /* */
            verbosePrint("Decoded:%d=%c\n",code.litteralValue,code.litteralValue);
            addElementToStream(decompressedStream, code.litteralValue);

        }
        else if(code.litteralValue == END_OF_BLOCK){
            /* If the code is a end of block code, then stop.*/
            break;
        } else if(code.litteralValue >= DISTANCE_MIN &&
                  code.litteralValue <= DISTANCE_MAX){
            verbosePrint("DECODE DISTANCE PAIR FOUND\n");

            decodeLengthDistancePair(
                code,
                distanceCodes,
                compressedStream,
                decompressedStream);
        } else{
            printError("Invalid code found:\n");
            printCode(code);
        }
    }

    decompressedStream = decompressedStream;
}

void decodeLengthDistancePair(
    HuffmanCode lengthCode,
    CodesList distanceCodes,
    DataStream * compressedStream,
    DataStream * decompressedStream)
{
    unsigned short fullLengthCode;

    unsigned short fullDistanceCode;

    HuffmanCode distanceCode;

    verbosePrint("length code:");
    printCode(lengthCode);
    verbosePrint("beginning length code:%d\n",lengthCode.litteralValue);

    fullLengthCode =  readRestOfLengthCode(
        lengthCode.litteralValue,
        compressedStream);

    verbosePrint("Real length code:%d\n",fullLengthCode);

    /* Read the following distance code. */
    distanceCode = readCode(distanceCodes, compressedStream);

    verbosePrint("distance code:");
    printCode(distanceCode);

    fullDistanceCode = readRestOfDistanceCode(distanceCode.litteralValue,compressedStream);

    verbosePrint("beginning distance code value:%d\n", distanceCode.litteralValue);
    verbosePrint("real distance code value:%d\n",fullDistanceCode);

    outputLengthDistancePair(fullLengthCode, fullDistanceCode, decompressedStream);
}

void outputLengthDistancePair(
    unsigned short lengthCode,
    unsigned short distanceCode,
    DataStream * decompressedStream)
{
    unsigned short i;
    BYTE toAdd;

    verbosePrint("Decoding length distance pair:\n");

    verbosePrint("length code:%d\n",lengthCode);

    for(i = 0; i < lengthCode; ++i){

        toAdd = decompressedStream->stream.data[decompressedStream->position - distanceCode];

        verbosePrint("Decoded:%d=%c\n",toAdd,toAdd);

        addElementToStream(decompressedStream, toAdd);
    }
}

unsigned short readRestOfDistanceCode(unsigned short code,DataStream * compressedStream)
{
    unsigned short distanceCode;
    DistanceTableEntry entry;

    entry = distanceTable[code];

    if(entry.extraBits != 0){
        distanceCode = entry.minDist + inputCodeLSB(entry.extraBits,compressedStream);
    } else{
        distanceCode = entry.minDist;
    }

    return distanceCode;
}

unsigned short readRestOfLengthCode(
    unsigned short code,
    DataStream * compressedStream)
{
    unsigned short lengthCode;
    LengthTableEntry entry;

    entry = lengthTable[code - 257];

    if(entry.extraBits != 0){
        lengthCode = entry.minLength + inputCodeLSB(entry.extraBits,compressedStream);
    } else{
        lengthCode = entry.minLength;
    }

    return lengthCode;
}

HuffmanCode readCode(CodesList codes,  DataStream * stream)
{
    int minimumCodeLength;
    unsigned short codeValue;
    int foundIndex;
    unsigned short codeLength;

    /* inefficient.  */
    minimumCodeLength = getMinimumCodeLength(codes);
    verbosePrint("minimum code length:%d\n",minimumCodeLength);

    /* read code of minimum value. */
    codeLength = minimumCodeLength;
    codeValue = inputCodeLSBRev(minimumCodeLength, stream);

    foundIndex = findCode(codes,codeValue,codeLength);

    while(foundIndex == -1){

        codeValue = appendBit(codeValue,stream);
        codeLength++;

        foundIndex = findCode(codes,codeValue,codeLength);
    }

    return codes.codes[foundIndex];
}


unsigned int inputCodeLSBRev(int codeSize, DataStream * stream)
{
    unsigned short code;
    int i;

    code = 0;

    for(i = 0; i < codeSize; ++i)
        code = appendBit(code,stream);

    verbosePrint("output reverse LSB code:%d\n",code);
    return code;
}

unsigned short appendBit(unsigned short codeValue, DataStream * stream)
{
    unsigned short bit;

    bit = inputCodeLSB(1,stream);

    return bit | (codeValue << 1);
}

int findCode(CodesList codes,unsigned short codeValue, unsigned short codeLength)
{
    int i;

    verbosePrint("codes size:%d",codes.size);
    for(i = 0; i < codes.size; ++i)
        if(codes.codes[i].codeValue == codeValue && codes.codes[i].codeLength == codeLength)
            break;

    if(i == codes.size){
        return -1;
    } else{
        verbosePrint("Found code with value %d and index %d\n",codeValue,i);
        return i;
    }
}

unsigned int inputCodeLSB(int codeSize, DataStream * stream)
{
    unsigned int returnValue;
    int shift;

    returnValue = 0;
    shift = 0;

    while(codeSize > 0){
        if(remainingPacketBits < codeSize){

            /* the data in the current byte are not enough bits of data. Reads in what's
               remaining and read a new byte. */

            /* read in what's left of the current byte */
            returnValue |=
                (firstNBits(stream->stream.data[stream->position],remainingPacketBits) << shift);
            /* increase the shift */
            shift += remainingPacketBits;
            codeSize -= remainingPacketBits;
            stream->position++;
            remainingPacketBits = 8;
            verbosePrint("Starting new byte: %x\n", stream->stream.data[stream->position]);

        }else{
            /* if remainingPacketBits > codeSize */
            /* Enough bits of data can be read from the current byte.
               Read in enough data and bitwise shift the data to the right to
               get rid of the bytes read in. */

            returnValue |=
                (firstNBits(stream->stream.data[stream->position], codeSize) << shift);

            stream->stream.data[stream->position] >>= codeSize;
            remainingPacketBits -= codeSize;

            /* enough bits of data has been read in. This line thus causes the loop to terminate
               and causes the functions to return. */
            codeSize = 0;
        }
    }

    verbosePrint("inputcodeLSB:%d\n",returnValue);
    return returnValue;
}

DEFLATE_BlockHeader readDEFLATE_BlockHeader(DataStream * stream)
{
    DEFLATE_BlockHeader header;

    /* This may not work for variably sized codes. */
    header.BFINAL = inputCodeLSB(1,stream);
    header.BTYPE = inputCodeLSB(2,stream);

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

void printCode(HuffmanCode code)
{
    int i;

    if(isprint(code.litteralValue))
	verbosePrint("Code literal value:%d = %c.",code.litteralValue,code.litteralValue);
    else
	verbosePrint("Code literal value:%d.",code.litteralValue);

    if(code.codeLength == 0){
        verbosePrint("zero length code.");
    }else{

        verbosePrint("Code Value:%d = ",code.codeValue);

        for(i = (code.codeLength - 1); i >= 0 ; --i)
            verbosePrint("%d", getBitToggled(code.codeValue,i));
    }
    verbosePrint(".\n");
}

int getBitToggled(unsigned short value,int bit)
{
    return (((value & (1 << bit)) >> bit));
}

CodesList translateCodes(BYTE * codeLengths, int alphabetSize)
{
    int * codeLengthFreqs;
    int * nextCode;
    int i;
    CodesList translatedCodes;

    unsigned short code;
    int maxBits;
    int bits;
    int len;

    /* Step 1. */
    codeLengthFreqs = getCodeLengthFreqs(codeLengths, alphabetSize);

    verbosePrint("Printing frequency table:\n");
    for(i = 0; i < alphabetSize; ++i)
        if(codeLengthFreqs[i] != 0)
            verbosePrint("N:%d | %d\n",i,codeLengthFreqs[i]);

    /* Step 2*/

    maxBits = 0;

    for(i = 1;i < alphabetSize; ++i)
        if(codeLengthFreqs[i] != 0)
            maxBits = i;

    /* enough memory? */
    nextCode = (int *)malloc(sizeof(int) * (maxBits + 1));

    code = 0;

    codeLengthFreqs[0] = 0;

    for(bits = 1; bits <= maxBits; ++bits){
        code = (code + codeLengthFreqs[bits - 1]) << 1;
        nextCode[bits] =  code;
    }

    verbosePrint("Printing next code:\n");
    for(i = 1; i <= maxBits; ++i){
        verbosePrint("N:%d | %d\n",i,nextCode[i]);
    }

    /* Step 3 */
    translatedCodes.size = alphabetSize;

    for(i = 0; i < alphabetSize; ++i){
        translatedCodes.codes[i].litteralValue = i;
        translatedCodes.codes[i].codeLength = 0;
    }

    for(i = 0; i < alphabetSize; ++i){

        len = codeLengths[i];

        if(len != 0){
            translatedCodes.codes[i].codeValue = nextCode[len];
            translatedCodes.codes[i].codeLength = len;

            nextCode[len]++;
        }

    }

    free(codeLengthFreqs);
    free(nextCode);

    return translatedCodes;
}

int * getCodeLengthFreqs(BYTE * codeLengths, int alphabetSize)
{
    int * freqs;
    int i;

    freqs = (int *)malloc(sizeof(int) * alphabetSize);

    for(i = 0; i < alphabetSize; ++i)
        freqs[i] = 0;

    for(i = 0; i < alphabetSize; ++i)
        ++freqs[codeLengths[i]];

    return freqs;
}

void translateCodesTest(void)
{
    int i;
    int alphabetLength = 8;
    BYTE codeLengths[8] = /*{2,1,3,3}; */

        {3,3,3,
         3,3,
         2,4,4};

    CodesList codes;

    codes = translateCodes(codeLengths, alphabetLength);


    for(i = 0; i < alphabetLength; ++i)
        printCode(codes.codes[i]);
}

void printCodesList(CodesList codes)
{
    int i;

    for(i = 0; i < codes.size; ++i)
        printCode(codes.codes[i]);

}
