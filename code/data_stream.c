#include "data_stream.h"
#include <math.h>

DataStream getNewDataStream(DataList list, int endian)
{
    DataStream stream;

    stream.list = list;
    stream.position = 0;
    stream.endian = endian;

    return stream;
}

BYTE readStreamByte(DataStream * stream)
{
    return *(BYTE *)stream->list.list[stream->position++];
}

uint16_t read16BitsNumber(DataStream * stream)
{
    BYTE b1,b2;

    b1 = readStreamByte(stream);
    b2 = readStreamByte(stream);

    return b1 * 256 + b2;
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
