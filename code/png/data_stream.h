#ifndef _DATA_STREAM_H_
#define _DATA_STREAM_H_

#include "fixed_data_list.h"

typedef struct {
    FixedDataList list;
    size_t position;
} DataStream;

DataStream getNewDataStream(FixedDataList list);

#endif /* _DATA_STREAM_H_ */
