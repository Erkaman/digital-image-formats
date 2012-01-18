#ifndef _DATA_STREAM_H_
#define _DATA_STREAM_H_

#include "data_list.h"
#include "bitwise.h"
#include "defs.h"

#define ENDIAN_BIG 1
#define ENDIAN_SMALL 0

typedef struct {
    DataList list;
    size_t position;

    int endian;

    int remainingBitsBits;
    BYTE b;

} DataStream;

DataStream getNewDataStream(DataList list, int endian);
BYTE readStreamByte(DataStream * stream);
uint16_t read16BitsNumber(DataStream * stream);
uint32_t read32BitsNumber(DataStream * stream);

void * readNext(DataStream * stream);

BYTE readBits(DataStream * stream, int nbits);

#endif /* _DATA_STREAM_H_ */
