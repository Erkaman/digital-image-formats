#ifndef _DATA_STREAM_H_
#define _DATA_STREAM_H_

#include "fixed_data_list.h"
#include "../common.h"
#include <stdint.h>

#define ENDIAN_BIG 1
#define ENDIAN_SMALL 0

typedef struct {
    FixedDataList list;
    size_t position;

    int endian;
} DataStream;

DataStream getNewDataStream(FixedDataList list, int endian);
BYTE readStreamByte(DataStream * stream);
uint16_t read16BitsNumber(DataStream * stream);
uint32_t read32BitsNumber(DataStream * stream);

#endif /* _DATA_STREAM_H_ */
