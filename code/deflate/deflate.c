#include "deflate.h"
#include "../common.h"
#include <stdlib.h>

BYTE * dataStream;
int dataI;

int remainingPacketBits;

BYTE getNextByte(void)
{
    return dataStream[++dataI];
}

DataContainer deflateDecompress(DataContainer data)
{
    DataContainer decompressedData;
    int lastBlock;
    DEFLATE_BlockHeader header;
    unsigned long i;
    BYTE b1;
    BYTE b2;

    dataStream = data.data;
    dataI = 0;
    remainingPacketBits = 8;

    lastBlock = 0;

    for(i = 0; i < data.size; ++i){
	verbosePrint("i:%d=%d\n",i,dataStream[i]);
    }

    do{
	header = readDEFLATE_BlockHeader();
	writeDEFLATE_BlockHeader(header);

	if(header.BTYPE == BTYPE_NO_COMPRESSION){
	    remainingPacketBits = 8;

	    /* FIXME: does this handle big and little endian? */
	    b1 = getNextByte();
	    b2 = getNextByte();

	    decompressedData = allocateDataContainer(b1 + b2 * 256);

	    /* Skip NLEN */
	    getNextByte();
	    getNextByte();

	    for(i = 0; i < decompressedData.size; ++i){
		decompressedData.data[i] = getNextByte();
	    }
	}
    }while(!header.BFINAL);

    return decompressedData;
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

    header.BFINAL = inputCodeLSB(1);
    header.BTYPE = inputCodeLSB(2);

    return header;
}

void writeDEFLATE_BlockHeader(DEFLATE_BlockHeader header)
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
