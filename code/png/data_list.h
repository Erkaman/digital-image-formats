#ifndef _DATA_LIST_H_
#define _DATA_LIST_H_

#include <stdlib.h>

typedef struct {

    void ** list;

    /* the number of elements in the list. */
    size_t count;

    /* the maximum number of elements in the list. */
    size_t maxCount;

    /* The size of all the individual elements. */
    size_t size;

} DataList;

#define DATA_LIST_GROWTH_FACTOR 2

DataList allocateNewDataList(size_t size, size_t count, size_t position);
void freeDataList(DataList list);
void printDataList(DataList list, void (*printFunc)(void *));
DataList getNewDataList(size_t size);
void print_size_t(void * s);
DataList accommodateDataListCount(DataList oldList, size_t newCount);
DataList growDataListCount(DataList oldList);
void addToDataList(DataList * list, void * newElement);


#endif /* _DATA_LIST_H_ */
