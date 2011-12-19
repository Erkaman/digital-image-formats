#include "data_stream.h"
#include <math.h>
#include "io.h"

DataStream getNewDataStream(DataList list, int endian)
{
    DataStream stream;

    stream.list = list;
    stream.position = 0;
    stream.endian = endian;

    stream.remainingBitsBits = 0;
    stream.b = 0;

    return stream;
}

BYTE readStreamByte(DataStream * stream)
{
    return *(BYTE *)stream->list.list[stream->position++];
}

uint16_t read16BitsNumber(DataStream * stream)
{
    BYTE b1,b2;
    uint16_t n;

    b1 = readStreamByte(stream);
    b2 = readStreamByte(stream);

    if(stream->endian == ENDIAN_BIG)
	n = b1 * 256 + b2;
    else if(stream->endian == ENDIAN_SMALL)
	n = b2 * 256 + b1;

/*    if(stream->endian == ENDIAN_BIG){
	n =  ntohs(n);

    } else if(stream->endian == ENDIAN_SMALL){

	n = htons(n);

    }*/

    return n;
}

uint32_t read32BitsNumber(DataStream * stream)
{
    BYTE b1, b2, b3, b4;

    b1 = readStreamByte(stream);
    b2 = readStreamByte(stream);
    b3 = readStreamByte(stream);
    b4 = readStreamByte(stream);

    return b1 * pow(256,3) + b2 * pow(256, 2) + b3 * pow(256,1) + b4 * pow(256,0);
}

BYTE readBits(DataStream * stream, int nbits)
{
    BYTE bits;

    /* Load next byte. */
    if(stream->remainingBitsBits == 0){
	stream->b = readStreamByte(stream);
	stream->remainingBitsBits = 8;
    }

    bits = lastNBits(stream->b, nbits, stream->remainingBitsBits) &
	(~(~0 << nbits));
    stream->remainingBitsBits -= nbits;

    verbosePrint("bits:%d\n",bits);
    return bits;
}
