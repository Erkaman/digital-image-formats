#ifndef _FIXED_DATA_LIST_H_
#define _FIXED_DATA_LIST_H_

#include <stdlib.h>

typedef struct {

    void ** list;

    size_t count;

} FixedDataList;

void freeFixedDataList(FixedDataList list);
void printFixedDataList(FixedDataList list, void (*printFunc)(void *));
FixedDataList getNewFixedDataList(size_t size, size_t count);


#endif /* _FIXED_DATA_LIST_H_ */
