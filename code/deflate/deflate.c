#include "deflate.h"
#include "../common.h"
#include <stdlib.h>

BYTE * dataStream;
int dataI;

HuffmanCode fixedHuffmanCodes[TREE_SIZE];

int remainingPacketBits;

int getMinimumCodeLength(HuffmanCode * codes)
{
    int min;
    int i;

    min = codes[0].codeLength;

    for(i = 1; i < TREE_SIZE; ++i)
        if(codes[i].codeLength < 0)
            min = codes[i].codeLength;

    return min;
}

BYTE getNextByte(void)
{
    return dataStream[++dataI];
}



void setFixedHuffmanCodes(void)
{
    int i;
    int codesI = 0;
    HuffmanCode code;

    verbosePrint("Assigning static huffman codes:\n");

    /* Litteral value 256 is the end code.*/

    /* Literal values 0-143 are given the codes
       00110000(=48=0x30) - 10111111(191=0xbf) of length 8*/
    for(i = 0; i <= 143; ++i){
        code.codeValue = 0x30 + i;
        code.litteralValue = i;
        code.codeLength = 8;
        fixedHuffmanCodes[codesI++] = code;

        printCode(code);
    }

    /* Literal values 144-255 are given the codes
       1 1001 0000(=400=0x190) - 1 1111 1111(=511=0x1ff) of length 9*/
    for(i = 144; i <= 255; ++i){
        code.codeValue = 0x190 + i - 144;
        code.litteralValue = i;
        code.codeLength = 9;
        fixedHuffmanCodes[codesI++] = code;

        printCode(code);
    }

    /* Literal values 256-279 are given the codes
       000 0000(=0=0x0) - 001 0111(=23=0x17) of length 7*/
    for(i = 256; i <= 279; ++i){
        code.codeValue = 0x0 + i - 256;
        code.litteralValue = i;
        code.codeLength = 7;
        fixedHuffmanCodes[codesI++] = code;

        printCode(code);
    }

    /* Literal values 280-287 are given the codes
       1100 0000(=192=0xc0) - 1100 0111(=199=0xc7) of length 8*/
    for(i = 280; i <= 287; ++i){
        code.codeValue = 0xc0 + i - 280;
        code.litteralValue = i;
        code.codeLength = 8;
        fixedHuffmanCodes[codesI++] = code;

        printCode(code);
    }
}

DataContainer deflateDecompress(DataContainer data)
{
    DataContainer decompressedData;
    int lastBlock;
    DEFLATE_BlockHeader header;
    unsigned long i;
    BYTE b1;
    BYTE b2;
    int decompressedI;

    unsigned long uncompressedBlockSize;

    unsigned short fixedCode;

    int foundIndex;

/*    unsigned short fixedCode; */

    /* Put this into a preparatory function? */
    setFixedHuffmanCodes();

    decompressedData = getEmptyDataContainer();

    dataStream = data.data;
    dataI = 2;
    remainingPacketBits = 8;

    lastBlock = 0;

    decompressedI = 0;

    decompressedData.size = 0;

    do{

        /* if dynaic codes are used a new table is read in before the decompression
           of the data. */
        header = readDEFLATE_BlockHeader();
        printDEFLATE_BlockHeader(header);

        if(header.BTYPE == BTYPE_NO_COMPRESSION){
            remainingPacketBits = 8;

            /* FIXME: does this handle big and little endian? */
            b1 = getNextByte();
            b2 = getNextByte();
            uncompressedBlockSize = (b1 + b2 * 256);

            decompressedData = accommodateDataContainer(decompressedData,

                                                        decompressedData.size + uncompressedBlockSize);

            /* Skip NLEN */
            getNextByte();
            getNextByte();

            for(i = 0; i < uncompressedBlockSize; ++i)
                decompressedData.data[decompressedI++] = getNextByte();
        } else if (header.BTYPE == BTYPE_COMPRESSED_FIXED_HUFFMAN_CODES){

            /* read code of minimum value. */
            fixedCode = inputCodeLSB(getMinimumCodeLength(fixedHuffmanCodes));

            foundIndex = findCode(fixedHuffmanCodes,fixedCode);

            while(foundIndex == -1){

		fixedCode = (fixedCode << 1) | inputCodeLSB(1);

		foundIndex = findCode(fixedHuffmanCodes,fixedCode);
            }

	    printCode(fixedHuffmanCodes[foundIndex]);

        }


    }while(header.BFINAL == 0);

    return decompressedData;
}


unsigned int inputCodeLSBRev(int codeSize)
{
    unsigned short code;
    int i;

    /* fixme this cod causes the codes to be build in wrong order. Fix so that they git build in the reverse order. */
    for(i = 0; i < codeSize; ++i){
        code = code | (inputCodeLSB(1) << codeSize);
    }

    verbosePrint("output reverse LSB code:%d\n",code);
    return code;


int findCode(HuffmanCode * codes, unsigned short codeValue)
{
    int i;

    for(i = 0; i < TREE_SIZE; ++i){
        if(codes[i].codeValue == codeValue){
            break;
        }
    }

    if(i == TREE_SIZE){
        return -1;
    } else{
        verbosePrint("Found code with value %d and index %d\n",codeValue,i);
        return i;
    }
}

unsigned int inputCodeLSB(int codeSize)
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
                (firstNBits(dataStream[dataI],remainingPacketBits) << shift);
            /* increase the shift */
            shift += remainingPacketBits;
            codeSize -= remainingPacketBits;
            dataI++;
            remainingPacketBits = 8;

        }else{
            /* if remainingPacketBits > codeSize */
            /* Enough bits of data can be read from the current byte.
               Read in enough data and bitwise shift the data to the right to
               get rid of the bytes read in. */

            returnValue |=
                (firstNBits(dataStream[dataI],codeSize) << shift);

            dataStream[dataI] >>= codeSize;
            remainingPacketBits -= codeSize;

            /* enough bits of data has been read in. This line thus causes the loop to terminate
               and causes the functions to return. */
            codeSize = 0;
        }
    }

    verbosePrint("inputcodeLSB::%d\n",returnValue);
    return returnValue;
}

DEFLATE_BlockHeader readDEFLATE_BlockHeader(void)
{
    DEFLATE_BlockHeader header;

    /* This may not work for variably sized codes. */
    if(dataI > 2){
        ++dataI;
    }
    header.BFINAL = inputCodeLSB(1);
    header.BTYPE = inputCodeLSB(2);

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
        verbosePrint("Reserved(error");
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

    verbosePrint("Code literal value:%d = %c.",code.litteralValue,code.litteralValue);

    verbosePrint("Code Value:%d = ",code.codeValue);

    for(i = (code.codeLength - 1); i >= 0 ; --i)
        verbosePrint("%d", getBitToggled(code.codeValue,i));

    verbosePrint(".\n");
}

int getBitToggled(unsigned short value,int bit)
{
    return (((value & (1 << bit)) >> bit));
}
